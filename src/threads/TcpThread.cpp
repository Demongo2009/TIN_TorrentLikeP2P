//
// Created by bartlomiej on 03.06.2021.
//
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
#include "../../include/threads/TcpThread.h"
#include "../../include/utils.h"

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
        close(tcpSocket);
        exit(1);
    }

    returnCode = listen(tcpSocket, 16);
    if (returnCode == -1){
        printf("Listen ERROR: %s\n", strerror(errno));
        close(tcpSocket);
        exit(1);
    }
    std::cout<<"Listening"<<std::endl;

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
    printf("waiting for client...\n");
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
    }
    else {
        std::cout<<"Client address: "<< inet_ntoa(clientAddr.sin_addr)<< "socket: "<<clientSocket<<std::endl;
    }
    connectedClients.insert(std::make_pair(clientSocket, clientAddr));
    for(auto& it: connectedClients){
        std::cout<<"connectedcleints"<<it.first<<"issync "<<it.second.isSync<<"addr "<<inet_ntoa(it.second.address.sin_addr)<<"port "<<htons(it.second.address.sin_port)<<std::endl;
    }
    return clientSocket;
}

void TcpThread::receive(int socket){
    char rbuf[MAX_MESSAGE_SIZE];
    memset(rbuf, 0, MAX_MESSAGE_SIZE);
    if (recv(socket, rbuf, sizeof(rbuf) - 1, 0) <= 0) {
        perror("receive error");
        if(connectedClients.find(socket)!= connectedClients.end())
            connectedClients.erase(socket);
        return;
    }

    printf("recv: %s\n", rbuf);

    char header[HEADER_SIZE];
    char payload[MAX_SIZE_OF_PAYLOAD];
    memset(header, 0, HEADER_SIZE);
    memset(payload, 0, MAX_SIZE_OF_PAYLOAD);
    snprintf(header, HEADER_SIZE , "%s", rbuf);
    snprintf(payload, MAX_SIZE_OF_PAYLOAD, "%s", rbuf+HEADER_SIZE);
    try {
        handleTcpMessage(header, payload, socket);
    }catch (std::exception& e ){
        std::cout<<"tcp catch: "<< e.what()<<std::endl;
    }

}

void TcpThread::runTcpServerThread() {

    initTcp();

    while (keepGoing) {
        std::cout << "TCP" << std::endl;
        int socket = acceptClient();
        if (socket > 0) {
            std::thread tcpWorker(&TcpThread::receive, this, socket);
            tcpWorker.detach();
        }
    }
}


void TcpThread::handleTcpMessage(char *header, char *payload, int socket) {
    std::cout<<"handletcp header "<<header<< " payload "<< payload;
    if(std::stoi(header) == DEMAND_CHUNK){
        std::cout<<"handletcp"<<std::endl;
        if(!connectedClients.at(socket).isSync){
            std::cout<<"syncing"<<std::endl;
            sendSync(socket, std::nullopt);
            if(receiveSync(socket, std::nullopt)) {
                connectedClients.at(socket).isSync = true;
            } else{
                std::cout<<"sync error"<<std::endl;
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
    sharedStructs.localResourcesMutex.lock();
    if(sharedStructs.localResources.find(message.resourceName) == sharedStructs.localResources.end() ) {
        sharedStructs.localResourcesMutex.unlock();
        return false;
    }
    long fileSize = sharedStructs.localResources.at(message.resourceName).sizeInBytes;
    int c = CHUNK_SIZE;
    for(const auto & index : message.chunkIndices) {
        long offset = index * c;
        if (offset > fileSize){//todo może >= nie chce mi się myśleć
            sharedStructs.localResourcesMutex.unlock();
            return false;
        }

    }
    sharedStructs.localResourcesMutex.unlock();
    return true;
}

void TcpThread::demandChunkJob(char *payload, int socket){
    DemandChunkMessage message = DemandChunkMessage::deserializeChunkMessage(payload);
    ResourceInfo resource;
    if(!validateChunkDemand(message)){
        std::cout<<"invlid chunk request"<< message.resourceName << message.chunkIndices[0] <<std::endl;
        sendHeader(socket, INVALID_CHUNK_REQUEST);
        close(socket);
        return;
    }
    sendChunks(message, socket);
    close(socket);

}


void TcpThread::sendChunks(const DemandChunkMessage& message, int socket){
    std::cout<<"sendchunks"<<std::endl;
    std::string filepath = sharedStructs.filepaths.at(message.resourceName);
    std::ifstream ifs {filepath, std::ios::in | std::ios_base::binary};
    sharedStructs.localResourcesMutex.lock();
    long fileSize = sharedStructs.localResources.at(message.resourceName).sizeInBytes;
    sharedStructs.localResourcesMutex.unlock();
    char chunk[CHUNK_SIZE];
    char sbuf[HEADER_SIZE + MAX_SIZE_OF_PAYLOAD] = {};
    int c = CHUNK_SIZE;
    for(const auto & index : message.chunkIndices) {
        long offset = index * c;
        ifs.seekg(offset, std::ios::beg);
        memset(chunk, 0, CHUNK_SIZE);
        if (offset + c <= fileSize) {
            ifs.read(chunk, CHUNK_SIZE);
        } else {
            std::cout<<"ile: "<<fileSize - offset << std::endl;
            ifs.read(chunk, fileSize - offset);
            chunk[fileSize - offset] = '\0';
        }
        memset(sbuf, 0, sizeof(sbuf));
        snprintf(sbuf, sizeof(sbuf), "%d;%d;%s", CHUNK_TRANSFER, index, chunk);
        std::cout<<"\n\n\n\n sbuf: "<< sbuf<<std::endl;
        if (send(socket, sbuf, sizeof sbuf, 0) < 0) {
            errno_abort("send chunk");
        }
    }

}

void TcpThread::sendHeader(int socket, TcpMessageCode code){
    char sbuf[HEADER_SIZE];
    memset(sbuf, 0, sizeof(sbuf));
    snprintf(sbuf, sizeof(sbuf), "%d", code);
    int n;
    if ( (n = send(socket, sbuf, sizeof sbuf, 0)) < 0 ) {
        errno_abort("send code error");
    }
    std::cout<<"SEND HEADER "<< sbuf<< "n: "<< n << std::endl;
}

bool TcpThread::receiveHeader(int socket, TcpMessageCode code){
    char header[HEADER_SIZE];
    memset(header, 0, HEADER_SIZE);
    if (recv(socket, header, sizeof(header), 0) <= 0) {
        std::cout<<"recverror"<<std::endl;
        perror("receive error");
        return false;
    }
    if(std::stoi(header) == code){
        return true;
    }
    return false;
}

void TcpThread::sendSync(int socket, std::optional<struct sockaddr_in> sockaddr){
    std::stringstream ss;
    sharedStructs.localResourcesMutex.lock();
    char payload[MAX_SIZE_OF_PAYLOAD] = {};
    char sbuf[HEADER_SIZE + MAX_SIZE_OF_PAYLOAD] = {};
    std::cout<<"sending"<<std::endl;
    for(const auto& [resourceName, resource] : sharedStructs.localResources){
        if(ss.str().size() + resourceName.size() > MAX_SIZE_OF_PAYLOAD){
            snprintf(payload, sizeof(payload), "%s", ss.str().c_str());
            memset(sbuf, 0 , sizeof(sbuf));
            snprintf(sbuf, sizeof(sbuf), "%d;%s", MY_STATE_BEFORE_FILE_TRANSFER, payload);
            if (send(socket, sbuf, strlen(sbuf) + 1, 0) < 0) {
                errno_abort("sync");
            }
            ss.clear();
            receiveHeader(socket, SYNC_OK);
        }
        ss << resource.resourceName  << ";"  << resource.revokeHash  << ";"  << resource.sizeInBytes  << ";";
    }

    sharedStructs.localResourcesMutex.unlock();
    memset(sbuf, 0 , sizeof(sbuf));
    memset(payload, 0 , sizeof(payload));
    snprintf(payload, sizeof(payload), "%s", ss.str().c_str());
    snprintf(sbuf, sizeof(sbuf), "%d;%s", MY_STATE_BEFORE_FILE_TRANSFER, payload);

    if (send(socket, sbuf, strlen(sbuf) + 1, 0) <= 0) {
        errno_abort("sync");
    }
    receiveHeader(socket, SYNC_OK);
    sendHeader(socket, SYNC_END);
    std::cout<<"sent: "<<sbuf<<std::endl;
}

void TcpThread::clearPeerInfo(struct sockaddr_in sockaddr){
    sharedStructs.networkResourcesMutex.lock();
    for(auto& it: connectedClients){
        std::cout<<"clearpeer"<<it.first<<"issync "<<it.second.isSync<<"addr "<<inet_ntoa(it.second.address.sin_addr)<<"issync "<<htons(it.second.address.sin_port)<<std::endl;
    }
    sharedStructs.networkResources.erase(convertAddressLong(sockaddr));
    sharedStructs.networkResourcesMutex.unlock();
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
    std::cout<<"receiving"<<std::endl;
    while (true) {
        memset(rbuf, 0, MAX_MESSAGE_SIZE);
        if (recv(socket, rbuf, sizeof(rbuf), 0) <= 0) {
            std::cout<<"recverror"<<std::endl;
            perror("receive error");
            return false;
        }

        memset(header, 0, HEADER_SIZE);
        snprintf(header, HEADER_SIZE , "%s", rbuf);
        std::cout<<"syncheader"<<header<<std::endl;
        if(std::stoi(header) == MY_STATE_BEFORE_FILE_TRANSFER) {
            std::cout<<"in"<<header<<std::endl;
            memset(payload, 0, MAX_SIZE_OF_PAYLOAD);
            snprintf(payload, sizeof(payload), "%s", rbuf + HEADER_SIZE);
            std::vector<ResourceInfo> resources = ResourceInfo::deserializeVectorOfResources(payload);
            sharedStructs.networkResourcesMutex.lock();
            for(const auto & r : resources){
                std::cout<<"reveivesync"<<std::endl;
                sharedStructs.networkResources[convertAddressLong(sockaddr)][r.resourceName] = r;
            }
            sharedStructs.networkResourcesMutex.unlock();
            sendHeader(socket, SYNC_OK);
        } else if(std::stoi(header) == SYNC_END){
            std::cout<<"in2"<<header<<std::endl;
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
