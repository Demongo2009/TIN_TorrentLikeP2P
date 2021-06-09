//
// Created by bartlomiej on 03.06.2021.
//
#include <thread>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <cmath>
#include <cassert>
#include "../../include/utils.h"
#include "../../include/threads/CliThread.h"

std::string CliThread::getUserPassword(){
    std::string prompt = "\nPlease input password:...\n";
    std::cout << prompt;

    std::string line;
    std::getline(std::cin, line);
    return line;
}


void CliThread::runCliThread() {
	pthread_barrier_wait(barrier);
    std::stringstream ss;
    std::string line;
    std::string prompt = "\nPlease input command:\n"
                         "new <filePath> <resourceName>\n"
                         "list\n"
                         "find <resourceName>\n"
                         "download <resourceName>\n"
                         "revoke <resourceName>\n"
                         "q (in order to quit)\n"
                         "Please input resourceNames shorter than " +  std::to_string(MAX_FILE_NAME_SIZE) + "\n";
    std::cout << prompt;
    std::getline(std::cin, line);
    std::vector<std::string> vecWord;

    while(keepGoing){
        ClientCommand parsedCommand;
        std::string userString, resourceName, password;
        std::cout<<"CLI"<<std::endl;
        ss << line;
        for(std::string s; ss >>s;){
            vecWord.push_back(s);
        }

        bool foundCommand= true;
        parsedCommand = parseCommand(vecWord, userString, resourceName, foundCommand);

        if(foundCommand){
            switch (parsedCommand) {
                case ADD_NEW_RESOURCE:
                    password = getUserPassword();

                    handleClientAddResource(resourceName, userString,password);
                    break;
                case LIST_AVAILABLE_RESOURCES:
                    handleClientListResources();
                    break;
                case FIND_RESOURCE:
                    handleClientFindResource(resourceName);
                    break;
                case DOWNLOAD_RESOURCE:
                    handleDownloadResource(resourceName, userString);
                    break;
                case REVOKE_RESOURCE:
                    password = getUserPassword();
                    handleRevokeResource(resourceName, password);
                    break;
                case EXIT:
					// clean before quit?

					keepGoing = false;
                    return;
            }
        }

        ss.clear();
        vecWord.clear();
        std::cout << prompt;
        std::getline(std::cin, line);
    }

}

void CliThread::terminate(){
    for(auto& it: openSockets){
        close(it);
    }
    for(auto & it: ongoingDownloadingFiles){
        remove(it.first.c_str());
    }
    std::cout<<"CLITERM"<<std::endl;
    keepGoing = false;
}

ClientCommand CliThread::parseCommand(std::vector<std::string> vecWord, std::string &filepath,
                                          std::string &resourceName, bool &foundCommand){
    ClientCommand parsedCommand = EXIT;
    if(vecWord[0] == "new") {
        parsedCommand = ADD_NEW_RESOURCE;

        if(vecWord.size() > 1){
            filepath = vecWord[1];
        }else{
            std::cout << "You must input file path!\n";
            foundCommand = false;
            return parsedCommand;
        }

        if(vecWord.size() > 2){
            resourceName = vecWord[2];
        }else{
            std::cout << "You must input file name!\n";
            foundCommand = false;
            return parsedCommand;
        }

        if(resourceName.size() > MAX_FILE_NAME_SIZE){
            std::cout << "File name too long! Has: " << resourceName.size() << "\n";
            foundCommand = false;
            return parsedCommand;
        }

    } else if(vecWord[0] == "list"){
        parsedCommand = LIST_AVAILABLE_RESOURCES;

    } else if(vecWord[0] == "find"){
        parsedCommand = FIND_RESOURCE;

        parseResourceName(vecWord, resourceName, foundCommand);


    } else if(vecWord[0] == "download"){
        parsedCommand = DOWNLOAD_RESOURCE;

        parseResourceName(vecWord, resourceName, foundCommand);


    } else if(vecWord[0] == "revoke"){
        parsedCommand = REVOKE_RESOURCE;

        parseResourceName(vecWord, resourceName, foundCommand);

    } else if(vecWord[0] == "q"){
        parsedCommand = EXIT;

    }else{
        std::cout << "Unrecognised command!\n";
        foundCommand = false;
    }

    return parsedCommand;
}

void CliThread::parseResourceName(std::vector<std::string> vecWord, std::string &resourceName, bool& foundCommand) const{
    if(vecWord.size() > 1){
        resourceName = vecWord[1];
    }else{
        std::cout << "You must input file name!\n";
        foundCommand = false;
        return;
    }

    if(resourceName.size() > MAX_FILE_NAME_SIZE){
        std::cout << "File name too long! Has: " << resourceName.size() << "\n";
        foundCommand = false;
    }
}


void CliThread::handleClientAddResource(const std::string& resourceName, const std::string& filepath,
                                            const std::string& userPassword) {
    std::ifstream f(filepath.c_str(), std::ios::binary | std::ios::ate);

    if(!f.good()){
        std::cout << "File doesnt exist in given file path!\n";
        return;
    }

    struct ResourceInfo resourceInfo;

    resourceInfo.resourceName = resourceName;
    resourceInfo.sizeInBytes = f.tellg();
    resourceInfo.revokeHash = std::hash<std::string >{}(userPassword);
    resourceInfo.isRevoked = false;

    sharedStructs.localResourcesMutex.lock();
    if(sharedStructs.localResources.find(resourceName) != sharedStructs.localResources.end()){
        std::cout << "File of this name already exists!\n";
        sharedStructs.localResourcesMutex.unlock();
        return;
    }
    for(auto& resources: sharedStructs.networkResources){
        auto it = resources.second.find(resourceName);
        if( it != resources.second.end()){
            std::cout << "File of this name already exists!\n";
            sharedStructs.localResourcesMutex.unlock();
            return;
        }
    }
    sharedStructs.localResources.emplace(resourceName, resourceInfo);
    sharedStructs.localResourcesMutex.unlock();
    sharedStructs.filepaths.insert(std::make_pair(resourceName, filepath));

    udpObj->broadcastNewFile(resourceInfo);
}

void CliThread::handleClientListResources() {
    std::cout<<"LOCAL RESOURCES: "<<std::endl;
    sharedStructs.localResourcesMutex.lock();
    for(const auto& it : sharedStructs.localResources){
        std::cout<< "NAME: "<<it.first<< " SIZE: "<<it.second.sizeInBytes<<std::endl;
    }
    sharedStructs.localResourcesMutex.unlock();

    std::cout<<"NETWORK RESOURCES: "<<std::endl;
    sharedStructs.networkResourcesMutex.lock();
    struct in_addr addr{};
    for(const auto& [peerAddress, resources] : sharedStructs.networkResources){
        addr.s_addr = peerAddress;
        std::cout<< "RESOURCES OF PEER: "<<inet_ntoa(addr) <<std::endl;
        for(const auto& it: resources){
            std::cout<< "NAME: "<<it.first<< " SIZE: "<<it.second.sizeInBytes<<std::endl;
        }
    }
    sharedStructs.networkResourcesMutex.unlock();
}


void CliThread::handleClientFindResource(const std::string& resourceName) {
    sharedStructs.localResourcesMutex.lock();
    auto it = sharedStructs.localResources.find(resourceName);
    if( it != sharedStructs.localResources.end()){
        std::cout<<"LOCAL RESOURCE"<< resourceName<< " PATH: " << sharedStructs.filepaths.at(resourceName) << std::endl;
    }
    sharedStructs.localResourcesMutex.unlock();
    sharedStructs.networkResourcesMutex.lock();
    struct in_addr addr{};
    for(auto& [peerAddress, resources]: sharedStructs.networkResources){
        it = resources.find(resourceName);
        if( it != resources.end()){
            addr.s_addr = peerAddress;
            std::cout<< "NETWORK RESOURCE OF PEER: "<<inet_ntoa(addr) <<std::endl;
            std::cout<<"NAME: "<< resourceName<< " SIZE: " << it->second.sizeInBytes << std::endl;
        }
    }
    sharedStructs.networkResourcesMutex.unlock();
}


void CliThread::handleDownloadResource(const std::string& resourceName, const std::string& filepath) {
    std::thread downloadThread(&CliThread::downloadResourceJob, this, resourceName, "filepath");
    downloadThread.detach();
}

void CliThread::downloadResourceJob(const std::string& resourceName, const std::string& filepath){
    sharedStructs.localResourcesMutex.lock();
    auto it = sharedStructs.localResources.find(resourceName);
    if( it != sharedStructs.localResources.end()){
        std::cout<<"ALREADY HAVE THE RESOURCE "<< resourceName<< " PATH: " << sharedStructs.filepaths.at(resourceName) << std::endl;
        sharedStructs.localResourcesMutex.unlock();
        return;
    }
    sharedStructs.localResourcesMutex.unlock();
    sharedStructs.networkResourcesMutex.lock();
    struct sockaddr_in addr{};
    std::vector<struct sockaddr_in> peersPossessingResource;
    unsigned long long fileSize;
    for(auto& [peerAddress, resources] : sharedStructs.networkResources){
        it = resources.find(resourceName);
        if( it != resources.end()){
            addr.sin_addr.s_addr = peerAddress;
            addr.sin_port = ntohs(5555);
            addr.sin_family = AF_INET;
            peersPossessingResource.emplace_back(addr);
            fileSize = it->second.sizeInBytes;
        }
    }
    sharedStructs.networkResourcesMutex.unlock();
    if(peersPossessingResource.empty()){
        std::cout<<"NONE IS IN POSSESSION OF THIS RESOURCE "<< resourceName<< std::endl;
        return;
    }
    std::cout<<"count peers"<<peersPossessingResource.size()<<std::endl;
    std::vector<std::vector<int> > chunkIndices = prepareChunkIndices(peersPossessingResource.size(), fileSize);
    std::cout<<"count chunk indices"<<chunkIndices.size()<<std::endl;
    for(auto& indices: chunkIndices){
        std::cout<<"PEER indices: "<<std::endl;
        for(auto& i : indices){
            std::cout<<"id: "<<i<<std::endl;
        }
    }
    std::vector<std::thread> threads;
    threads.reserve(chunkIndices.size());
    SynchronizedFile file = SynchronizedFile(filepath);
    file.reserveFile(fileSize);
    ongoingDownloadingFiles.insert(std::make_pair(filepath, file));
    for(int i = 0; i < chunkIndices.size(); ++i){
        threads.emplace_back(&CliThread::downloadChunksFromPeer, this, peersPossessingResource[i], chunkIndices[i], resourceName, filepath);
    }
    for(auto & thread: threads){
        thread.join();
    }
    ongoingDownloadingFiles.erase(filepath);
    ResourceInfo downloadedResource = ResourceInfo(resourceName, fileSize);
    sharedStructs.localResourcesMutex.lock();
    sharedStructs.localResources[resourceName] = downloadedResource;
    sharedStructs.localResourcesMutex.unlock();
    sharedStructs.filepaths[resourceName] = filepath;
    udpObj->broadcastNewFile(downloadedResource);
}

void CliThread::downloadChunksFromPeer( struct sockaddr_in sockaddr, const std::vector<int>& chunksIndices, const std::string& resourceName, const std::string &filepath){

    std::stringstream ss;
    char payload[MAX_SIZE_OF_PAYLOAD] = {};
    char sbuf[MAX_MESSAGE_SIZE] = {};
    bool first = true;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(connect(sock, (struct sockaddr *) &sockaddr, sizeof sockaddr) < 0){
        throw std::runtime_error("connect fail");
    }
    openSockets.insert(sock);
    int chunksCount = 0;
    for(const auto& index : chunksIndices){
        if(ss.str().size() + std::to_string(index).size() > MAX_SIZE_OF_PAYLOAD){
            memset(payload, 0 , sizeof(payload));
            snprintf(payload, sizeof(payload), "%s", ss.str().c_str());
            memset(sbuf, 0 , sizeof(sbuf));
            snprintf(sbuf, sizeof(sbuf), "%d;%s;%s", DEMAND_CHUNK, resourceName.c_str(), payload);
            if (send(sock, sbuf, strlen(sbuf) + 1, 0) < 0) {
                errno_abort("send");
            }
            ss.clear();
            if(first){
                tcpObj->receiveSync(sock, sockaddr);
                tcpObj->sendSync(sock);
                first = false;
            }
            receiveChunks(sock, chunksCount, filepath);
            chunksCount = 0;
        }
        ++chunksCount;
        ss<< index << ";" ;
        std::cout<<"STRINGS: " << ss.str() <<std::endl;

    }
    memset(sbuf, 0 , sizeof(sbuf));
    memset(payload, 0 , sizeof(payload));
    snprintf(payload, sizeof(payload), "%s", ss.str().c_str());
    snprintf(sbuf, sizeof(sbuf), "%d;%s;%s", DEMAND_CHUNK, resourceName.c_str(), payload);
    if (send(sock, sbuf, strlen(sbuf) + 1, 0) < 0) {
        errno_abort("send");
    }
    if(first){
        tcpObj->receiveSync(sock,sockaddr);
        tcpObj->sendSync(sock);
        std::cout<<"poszlo"<<std::endl;
    }
    unsigned long long size = sharedStructs.networkResources[sockaddr.sin_addr.s_addr].at(resourceName).sizeInBytes;
    receiveChunks(sock, chunksCount, filepath);
    close(sock);
    openSockets.erase(sock);
}


void CliThread::receiveChunks(int sock, int chunksCount, const std::string &filepath) {
    char rbuf[MAX_MESSAGE_SIZE];
    for(int i = 0; i < chunksCount; ++i) {
        memset(rbuf, 0, MAX_MESSAGE_SIZE);
        if (recv(sock, rbuf, sizeof(rbuf), 0) < 0) {
            perror("receive error");
            exit(EXIT_FAILURE);
        }
        ChunkTransfer message = ChunkTransfer::deserializeChunkTransfer(rbuf);
        std::cout<<"\n\n\n\n\n\n\n"<<message.header << "  "<< message.index << "payload: " << message.payload<< std::endl;
        if (message.header == CHUNK_TRANSFER) {
            ongoingDownloadingFiles.at(filepath).write(message.payload, message.index);

        } else {
            throw std::runtime_error("receive chunks bad header");
        }

    }
}

std::vector<std::vector<int> > CliThread::prepareChunkIndices(int peersCount, unsigned int fileSize){
    std::vector<std::vector<int> > chunkIndices;
    int c = CHUNK_SIZE;
    int f = fileSize;
    int chunks = ceil((double) f / c );
    for(int i = 0; i < peersCount && i < chunks; ++i){
        chunkIndices.emplace_back(std::vector<int>());
    }

    for(int i = 0; i < chunks; ++i){
        chunkIndices[i%peersCount].emplace_back(i);
    }
    return chunkIndices;
}

void CliThread::handleRevokeResource(const std::string& resourceName, const std::string& userPassword) {
    sharedStructs.localResourcesMutex.lock();
    std::size_t hash = std::hash<std::string>{}(userPassword);
    if(sharedStructs.localResources.find(resourceName) == sharedStructs.localResources.end()){
        sharedStructs.localResourcesMutex.unlock();
        std::cout<<"No such resource"<<std::endl;
        return;
    }
    if(sharedStructs.localResources.at(resourceName).revokeHash != hash ){
        sharedStructs.localResourcesMutex.unlock();
        std::cout<<"You are not an original owner of this resource"<<std::endl;
        return;
    }
    sharedStructs.localResources.erase(resourceName);
    sharedStructs.localResourcesMutex.unlock();
    sharedStructs.networkResourcesMutex.lock();
    for(auto& resources: sharedStructs.networkResources){
        auto it = resources.second.find(resourceName);
        if( it != resources.second.end()){
            resources.second.erase(resourceName);
        }
    }
    sharedStructs.networkResourcesMutex.unlock();

    udpObj->broadcastRevokeFile(resourceName);

}

void CliThread::setBarrier(pthread_barrier_t *ptr) {
	barrier = ptr;
}
