#include <thread>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <cmath>
#include <future>
#include <cassert>
#include "../../include/utils/utils.h"
#include "../../include/threads/CliThread.h"
#include "../../include/utils/CliParser.h"

void CliThread::runCliThread() {
    const std::string menu = "Available options:\n"
                             "-new <filePath> <resourceName>\n"
                             "-list\n"
                             "-find <resourceName>\n"
                             "-download <resourceName> <filePath>\n"
                             "-revoke <resourceName>\n"
                             "-q (in order to quit)\n"
                             "Maximum resource name length=" +  std::to_string(MAX_FILE_NAME_SIZE) +
                             "\n======================================\n";

    pthread_barrier_wait(barrier);
    std::stringstream ss;
    std::string wholeCommand;


    std::cout<<menu;
    std::vector<std::string> commandWords;

    while(keepGoing){
        std::cout << "Please input command:\n";
        std::getline(std::cin, wholeCommand);

        ClientCommand parsedCommand;
        std::string filepath, resourceName;
        ss << wholeCommand;
        for(std::string s; ss >>s;){
            commandWords.push_back(s);
        }
        parsedCommand = CliParser::parseCommand(commandWords, filepath, resourceName);

        switch (parsedCommand) {
            case ADD_NEW_RESOURCE:
                handleClientAddResource(resourceName, filepath);
                break;
            case LIST_AVAILABLE_RESOURCES:
                handleClientListResources();
                break;
            case FIND_RESOURCE:
                handleClientFindResource(resourceName);
                break;
            case DOWNLOAD_RESOURCE:
                handleDownloadResource(resourceName, filepath);
                break;
            case REVOKE_RESOURCE:
                handleRevokeResource(resourceName);
                break;
            case EXIT:
                keepGoing = false;
                exit(0);
            case UNKNOWN:
                std::cout << "Unrecognised command!\n";
                break;
        }
        ss.clear();
        commandWords.clear();
    }

}

void CliThread::terminate(){
    for(auto& it: openSockets){
        close(it);
    }
    for(auto & it: ongoingDownloadingFiles){
        remove(it.first.c_str());
    }
    keepGoing = false;
}

void CliThread::handleClientAddResource(const std::string& resourceName, const std::string& filepath) {

    std::string userPassword = getResourcePassword();
    std::ifstream f(filepath.c_str(), std::ios::binary | std::ios::ate);

    if(!f.good()){
        std::cout << "File doesn't exist in a given file path!\n";
        return;
    }

    struct ResourceInfo resourceInfo;

    resourceInfo.resourceName = resourceName;
    resourceInfo.sizeInBytes = f.tellg();
    resourceInfo.revokeHash = std::hash<std::string >{}(userPassword);
    resourceInfo.isRevoked = false;
    if(sharedStructs.addLocalResource(resourceInfo, filepath, true)){
        udpObj->broadcastNewFile(resourceInfo);
    }
}

void CliThread::handleClientListResources() {
    std::cout<<"LOCAL RESOURCES: "<<std::endl;
    sharedStructs.printLocalResources();

    std::cout<<std::endl<<"NETWORK RESOURCES: "<<std::endl;
    sharedStructs.printNetworkResources();

    std::cout<<std::endl;
}


void CliThread::handleClientFindResource(const std::string& resourceName) {
    sharedStructs.findLocalResource(resourceName);
    sharedStructs.findNetworkResource(resourceName);
}


void CliThread::handleDownloadResource(const std::string& resourceName, const std::string& filepath) {
    std::thread downloadThread(&CliThread::downloadResourceJob, this, resourceName, filepath);
    downloadThread.detach();
}

void CliThread::downloadResourceJob(const std::string& resourceName, const std::string& filepath){
    if(!sharedStructs.checkIfFileIsInPossession(resourceName)){
        return;
    }
    std::vector<struct sockaddr_in> peersPossessingResource =
            sharedStructs.getNodesContainingResource(resourceName);

    if(peersPossessingResource.empty()){
        std::cout<<"NOBODY IS IN POSSESSION OF THE RESOURCE: "<< resourceName<< std::endl;
        return;
    }

    //ugly, but correct, since we already check that there is at least one peer
    unsigned long long fileSize = sharedStructs
            .networkResources
            .at(peersPossessingResource[0].sin_addr.s_addr)
            .at(resourceName).sizeInBytes;
    std::size_t revokeHash = sharedStructs
            .networkResources
            .at(peersPossessingResource[0].sin_addr.s_addr)
            .at(resourceName).revokeHash;

    std::vector<std::vector<int> > chunkIndices =
            prepareChunkIndices(peersPossessingResource.size(), fileSize);

    std::vector<std::future<bool> > threads;
    threads.reserve(chunkIndices.size());
    SynchronizedFile file = SynchronizedFile(filepath);
    file.reserveFile(fileSize);
    ongoingDownloadingFiles.insert(std::make_pair(filepath, file));
    for(int i = 0; i < chunkIndices.size(); ++i){
        threads.emplace_back(std::async(&CliThread::downloadChunksFromPeer, this, peersPossessingResource[i], chunkIndices[i], resourceName, filepath));
    }
    bool isSuccessful = true;
    for(auto & thread: threads){
        if(!thread.get()){
            isSuccessful = false;
        }
    }
    ongoingDownloadingFiles.erase(filepath);
    if(isSuccessful) {
        ResourceInfo downloadedResource = ResourceInfo(resourceName, fileSize, revokeHash);
        sharedStructs.addLocalResource(downloadedResource, filepath, false);
        udpObj->broadcastNewFile(downloadedResource);
    }

}

bool CliThread::downloadChunksFromPeer( struct sockaddr_in sockaddr, const std::vector<int>& chunksIndices, const std::string& resourceName, const std::string &filepath){

    std::stringstream ss;
    char payload[MAX_SIZE_OF_PAYLOAD] = {};
    char sbuf[MAX_MESSAGE_SIZE] = {};
    bool first = true;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(connect(sock, (struct sockaddr *) &sockaddr, sizeof sockaddr) < 0){
        return false;
    }
    try {
        openSockets.insert(sock);
        int chunksCount = 0;
        for (const auto &index : chunksIndices) {
            if (ss.str().size() + 2 * std::to_string(index).size() + resourceName.size() + 1 >= MAX_SIZE_OF_PAYLOAD) {
                memset(payload, 0, sizeof(payload));
                snprintf(payload, sizeof(payload), "%s", ss.str().c_str());
                memset(sbuf, 0, sizeof(sbuf));
                snprintf(sbuf, sizeof(sbuf), "%d;%s;%s", DEMAND_CHUNK, resourceName.c_str(), payload);
                if (send(sock, sbuf, strlen(sbuf) + 1, 0) < 0) {
                    errno_abort("send");
                }
                ss.str("");
                if (first) {
                    tcpObj->receiveSync(sock, sockaddr);
                    tcpObj->sendSync(sock);
                    first = false;
                }
                receiveChunks(sock, chunksCount, filepath);

                chunksCount = 0;
            }
            ++chunksCount;
            ss << index << ";";
        }
        memset(sbuf, 0, sizeof(sbuf));
        memset(payload, 0, sizeof(payload));
        snprintf(payload, sizeof(payload), "%s", ss.str().c_str());
        snprintf(sbuf, sizeof(sbuf), "%d;%s;%s", DEMAND_CHUNK, resourceName.c_str(), payload);
        if (send(sock, sbuf, strlen(sbuf) + 1, 0) < 0) {
            errno_abort("send");
        }
        if (first) {
            tcpObj->receiveSync(sock, sockaddr);
            tcpObj->sendSync(sock);
        }
        receiveChunks(sock, chunksCount, filepath);
        close(sock);
        openSockets.erase(sock);
        return true;
    }catch (std::exception& e ){
        close(sock);
        openSockets.erase(sock);
        return false;
    }
}


void CliThread::receiveChunks(int sock, int chunksCount, const std::string &filepath) {
    uint8_t rbuf[MAX_MESSAGE_SIZE];
    unsigned long long fileSize;
    for(int i = 0; i < chunksCount; ++i) {
        memset(rbuf, 0, MAX_MESSAGE_SIZE);

        if (recv(sock, rbuf, sizeof(rbuf), 0) < 0) {
            perror("receive error");
            exit(EXIT_FAILURE);
        }

        fileSize = ongoingDownloadingFiles.at(filepath).getSize();
        std::optional<ChunkTransfer> messageOpt = ChunkTransfer::deserializeChunkTransfer(rbuf, fileSize);
        if (messageOpt.has_value()){
            ChunkTransfer message = messageOpt.value();
            tcpObj->sendHeader(sock, CHUNK_TRANSFER_OK);
            unsigned int countBytesToWrite = getCountBytesToWrite(fileSize, message.index, CHUNK_SIZE);
            ongoingDownloadingFiles.at(filepath).write(message.payload, message.index, countBytesToWrite);
        }else {
            throw std::runtime_error("receive chunks bad header");
        }

    }
}

std::vector<std::vector<int> > CliThread::prepareChunkIndices(int peersCount, unsigned int fileSize){
    std::vector<std::vector<int> > chunkIndices;
    int c = CHUNK_SIZE;
    unsigned int f = fileSize;
    int chunks = ceil((double) f / c );
    for(int i = 0; i < peersCount && i < chunks; ++i){
        chunkIndices.emplace_back(std::vector<int>());
    }

    for(int i = 0; i < chunks; ++i){
        chunkIndices[i%peersCount].emplace_back(i);
    }
    return chunkIndices;
}

void CliThread::handleRevokeResource(const std::string& resourceName) {
    std::string userPassword = getResourcePassword();
    std::size_t hash = std::hash<std::string>{}(userPassword);

    if(sharedStructs.deleteLocalResourceAfterRevoke(resourceName, hash)){
        sharedStructs.deleteNetworkResource(resourceName);
        udpObj->broadcastRevokeFile(resourceName);
    }
}

void CliThread::setBarrier(pthread_barrier_t *ptr) {
	barrier = ptr;
}


std::string CliThread::getResourcePassword(){
    std::cout <<  "\nPlease input password:...\n";

    std::string line;
    std::getline(std::cin, line);
    return line;
}
