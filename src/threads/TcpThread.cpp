#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <string>
#include <fstream>
#include <cmath>
#include <vector>
#include <cstdio>
#include "../../include/threads/TcpThread.h"
#include "../../include/utils/utils.h"

void TcpThread::initTcp(){
    struct sockaddr_in serverAddr{};

    tcpSocket = socket(PF_INET, SOCK_STREAM, 0);
    myAddress = getMyAddress(tcpSocket);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(myAddress.c_str());
    memset(serverAddr.sin_zero, 0, sizeof serverAddr.sin_zero);
    int returnCode = bind(tcpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    if (returnCode == -1) {
        printf("BIND ERROR: %s\n", strerror(errno));
    }

    returnCode = listen(tcpSocket, 16);
    if (returnCode == -1){
        printf("Listen ERROR: %s\n", strerror(errno));
        close(tcpSocket);
        exit(1);
    }
    printTcpThreadMessage("Listening");

	pthread_barrier_wait(barrier);
}

void TcpThread::terminate(){
    keepGoing = false;
    shutdown(tcpSocket, SHUT_RDWR);
    close(tcpSocket);
    for(auto& it: connectedClients){
        close(it.first);
    }
}

int TcpThread::acceptClient() {
    printTcpThreadMessage("waiting for client...");
    struct sockaddr_in clientAddr{};
    socklen_t size = sizeof(clientAddr);
    int clientSocket = accept(tcpSocket, (struct sockaddr *) &clientAddr, &size);

    if (clientSocket == -1){
        printf("ACCEPT ERROR: %s\n", strerror(errno));
        return -1;
    }

    int currentReturnCode = getpeername(clientSocket, (struct sockaddr *) &clientAddr, &size);
    if (currentReturnCode == -1){
        printf("GETPEERNAME ERROR: %s\n", strerror(errno));
        return -1;
    }else {
        std::cout<<"Client address: "<< inet_ntoa(clientAddr.sin_addr)<< "socket: "<<clientSocket<<std::endl;
    }
    connectedClients.insert(std::make_pair(clientSocket, clientAddr));

    return clientSocket;
}

void TcpThread::receive(int socket){
    char rbuf[MAX_MESSAGE_SIZE];
    memset(rbuf, 0, MAX_MESSAGE_SIZE);
    if (recv(socket, rbuf, sizeof(rbuf) - 1, 0) <= 0) {
        if(connectedClients.find(socket)!= connectedClients.end()) {
            connectedClients.erase(socket);
            close(socket);
        }
        return;
    }

    printTcpThreadMessage(std::string("recv: %s\n") + rbuf);

    char header[HEADER_SIZE];
    char payload[MAX_SIZE_OF_PAYLOAD];
    memset(header, 0, HEADER_SIZE);
    memset(payload, 0, MAX_SIZE_OF_PAYLOAD);
    snprintf(header, HEADER_SIZE , "%s", rbuf);
    snprintf(payload, MAX_SIZE_OF_PAYLOAD, "%s", rbuf+HEADER_SIZE);
    try {
        handleTcpMessage(header, payload, socket);
    }catch (std::exception& e ){
        printTcpThreadMessage(std::string("tcp catch: ") + e.what());
    }
}

void TcpThread::runTcpServerThread() {
    initTcp();

    while (keepGoing) {
        int socket = acceptClient();
        if (socket > 0) {
            std::thread tcpWorker(&TcpThread::receive, this, socket);
            tcpWorker.detach();
        }
    }
}


void TcpThread::handleTcpMessage(char *header, char *payload, int socket) {
    if(std::stoi(header) == DEMAND_CHUNK){
        if(!connectedClients.at(socket).isSync){
            sendSync(socket);
            if(receiveSync(socket, std::nullopt)) {
                connectedClients.at(socket).isSync = true;
            } else{
                printTcpThreadMessage("sync error");
                return;
            }
        }
        demandChunkJob(payload, socket);
        receive(socket);
        connectedClients.erase(socket);
    }else{
        throw std::runtime_error("bad tcp header received");
    }

}

bool TcpThread::validateChunkDemand(const DemandChunkMessage& message){
    std::string resourceName = message.resourceName;
    std::vector<unsigned long> chunkIndices = message.chunkIndices;
    return sharedStructs.validateChunks(resourceName, chunkIndices);
}

void TcpThread::demandChunkJob(char *payload, int socket){
    DemandChunkMessage message = DemandChunkMessage::deserializeChunkMessage(payload);
    ResourceInfo resource;
    if(!validateChunkDemand(message)){
        std::cout<<"invalid chunk request"<< message.resourceName << message.chunkIndices[0] <<std::endl;
        sendHeader(socket, INVALID_CHUNK_REQUEST);
        close(socket);
        return;
    }
    sendChunks(message, socket);
}


void TcpThread::sendChunks(const DemandChunkMessage& message, int socket){
    std::string filepath = sharedStructs.filepaths.at(message.resourceName);
    std::ifstream ifs {filepath, std::ios::in };

    unsigned long fileSize = sharedStructs.getFileSize(message.resourceName);

    uint8_t chunk[CHUNK_SIZE];
    char sbuf[MAX_MESSAGE_SIZE] = {};
    int c = CHUNK_SIZE;
    unsigned int nToWrite;
    for(const auto & index : message.chunkIndices) {
        unsigned long offset = index * c;
        ifs.seekg(offset, std::ios::beg);

        memset(chunk, 0, CHUNK_SIZE);
        if (offset + c <= fileSize) {
            ifs.read((char *)chunk, CHUNK_SIZE);
            nToWrite = c;
        } else {
            ifs.read((char *)chunk, fileSize - offset);
            chunk[fileSize - offset] = '\0';
            nToWrite = fileSize - offset;
        }
        memset(sbuf, 0, sizeof(sbuf));
        snprintf(sbuf, sizeof(sbuf), "%d;%lu;", CHUNK_TRANSFER, index);
        memcpy(sbuf + std::to_string(CHUNK_TRANSFER).size() + std::to_string(index).size() + 2, chunk, nToWrite);
        if (send(socket, sbuf, MAX_MESSAGE_SIZE, 0) < 0) {
            errno_abort("send chunk");
        }
        receiveHeader(socket, CHUNK_TRANSFER_OK);
    }
    ifs.close();
}

void TcpThread::sendHeader(int socket, TcpMessageCode code){
    char sbuf[HEADER_SIZE];
    memset(sbuf, 0, sizeof(sbuf));
    snprintf(sbuf, sizeof(sbuf), "%d", code);

    if ( send(socket, sbuf, sizeof sbuf, 0) < 0 ) {
        errno_abort("send code error");
    }
}

bool TcpThread::receiveHeader(int socket, TcpMessageCode code){
    char header[HEADER_SIZE];
    memset(header, 0, HEADER_SIZE);
    if (recv(socket, header, sizeof(header), 0) <= 0) {
        close(socket);
        this->connectedClients.erase(socket);

        throw std::runtime_error("receive header failed");
    }
    if(std::stoi(header) == code){
        return true;
    }
    printTcpThreadMessage(std::string("bad receive header: ") + header);
    return false;
}

void TcpThread::sendSync(int socket){
    char payload[MAX_SIZE_OF_PAYLOAD] = {};
    char sbuf[HEADER_SIZE + MAX_SIZE_OF_PAYLOAD] = {};
    std::vector<std::string> messagePayloads = sharedStructs.getLocalStateString();

    for(const auto& payloadStr: messagePayloads) {
        memset(sbuf, 0 , sizeof(sbuf));
        memset(payload, 0 , sizeof(payload));
        snprintf(payload, sizeof(payload), "%s", payloadStr.c_str());
        snprintf(sbuf, sizeof(sbuf), "%d;%s", MY_STATE_BEFORE_FILE_TRANSFER, payload);
        if (send(socket, sbuf, strlen(sbuf) + 1, 0) < 0) {
                errno_abort("sync");
        }
        receiveHeader(socket, SYNC_OK);
    }
    sendHeader(socket, SYNC_END);
    printTcpThreadMessage(std::string("sent: ") + sbuf);
}

void TcpThread::clearPeerInfo(struct sockaddr_in sockaddr){
    sharedStructs.deleteNetworkNode(sockaddr);
}

bool TcpThread::receiveSync(int socket, std::optional<struct sockaddr_in> sockaddrOpt ){
    struct sockaddr_in sockaddr;
    if(sockaddrOpt.has_value()){
        sockaddr = sockaddrOpt.value();
    }else{
        sockaddr = connectedClients.at(socket).address;
    }
    clearPeerInfo(sockaddr);
    char rbuf[MAX_MESSAGE_SIZE];
    char header[HEADER_SIZE];
    char payload[MAX_SIZE_OF_PAYLOAD];
    while (true) {
        memset(rbuf, 0, MAX_MESSAGE_SIZE);
        if (recv(socket, rbuf, sizeof(rbuf), 0) <= 0) {
            perror("receive error");
            return false;
        }

        memset(header, 0, HEADER_SIZE);
        snprintf(header, HEADER_SIZE , "%s", rbuf);
        if(std::stoi(header) == MY_STATE_BEFORE_FILE_TRANSFER) {
            memset(payload, 0, MAX_SIZE_OF_PAYLOAD);
            snprintf(payload, sizeof(payload), "%s", rbuf + HEADER_SIZE);
            std::vector<ResourceInfo> resources = ResourceInfo::deserializeVectorOfResources(payload);

            sharedStructs.registerNewNodeWithItsResources(sockaddr, resources);

            sendHeader(socket, SYNC_OK);
        } else if(std::stoi(header) == SYNC_END){
            return true;
        }else{
            //invalid chunk request
            throw std::runtime_error("receive sync bad header");
        }

    }

}

void TcpThread::setBarrier(pthread_barrier_t *ptr) {
	barrier = ptr;
}

void TcpThread::printTcpThreadMessage(std::string message) {
    std::cout<<"\t TCP THREAD:" <<message<<std::endl;
}
