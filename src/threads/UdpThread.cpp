//
// Created by bartlomiej on 03.06.2021.
//
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <string>
#include <fstream>

#include <vector>
#include "../../include/utils.h"
#include "../../include/threads/UdpThread.h"


void UdpThread::handleUdpMessage(char *header, char *payload, sockaddr_in sockaddr) {
    switch (std::stoi(header)) {
        case NEW_RESOURCE_AVAILABLE:
            handleNewResourceAvailable(payload, sockaddr);
            break;
        case OWNER_REVOKED_RESOURCE:
            handleOwnerRevokedResource(payload, sockaddr);
            break;
        case NODE_DELETED_RESOURCE:
            handleNodeDeletedResource(payload, sockaddr);
            break;
        case NEW_NODE_IN_NETWORK:
            handleNewNodeInNetwork(sockaddr);
            break;
        case STATE_OF_NODE:
            handleStateOfNode(payload, sockaddr);
            break;
        case NODE_LEFT_NETWORK:
            handleNodeLeftNetwork(sockaddr);
            break;
    }
}

void UdpThread::runUdpServerThread() {

    initUdp();
	pthread_barrier_wait(barrier);
    broadcastNewNode();
    while (keepGoing){
        receive();
    }
}

void UdpThread::terminate(){
    keepGoing = false;
    broadcastLogout();
    close(udpSocket);
    exit(0);
}

void UdpThread::receive(){
    char rbuf[MAX_MESSAGE_SIZE];
    memset(rbuf, 0, MAX_MESSAGE_SIZE);
    struct sockaddr_in clientAddr{};
    socklen_t clientLength = sizeof(sockaddr_in);
//    std::string input = "tak;13399626275067451483;38";
//	handleNewResourceAvailable(const_cast<char *>(input.c_str()), clientAddr);
    if (recvfrom(udpSocket, rbuf, sizeof(rbuf) - 1, 0,(struct sockaddr *) &clientAddr, &clientLength) < 0) {
        perror("receive error");
        exit(EXIT_FAILURE);
    }
    clientAddr.sin_port = (in_port_t) htons(port);
    printf("recv: %s\n", rbuf);

    char header[HEADER_SIZE];
    char payload[MAX_SIZE_OF_PAYLOAD];
    memset(header, 0, HEADER_SIZE);
    memset(payload, 0, MAX_SIZE_OF_PAYLOAD);
    snprintf(header, HEADER_SIZE, "%s", rbuf);
    snprintf(payload, MAX_SIZE_OF_PAYLOAD, "%s", rbuf+HEADER_SIZE);
    std::string clientAddressString = inet_ntoa(clientAddr.sin_addr);
    std::cout<<"header: "<< header << " payload: " << payload << "\n";
    std::cout<<"clientaddr: "<< clientAddressString << " port: " << htons (clientAddr.sin_port) << "\n";
    if(clientAddressString != myAddress && clientAddressString != "127.0.0.1"){
        handleUdpMessage(header, payload, clientAddr);
    }

}

void UdpThread::initUdp() {

    int trueFlag = 1;
    if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        errno_abort("socket");
    }
    myAddress = getMyAddress(udpSocket);

    if (setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof trueFlag) < 0) {
        errno_abort("setsockopt");
    }

    memset(&recv_addr, 0, sizeof recv_addr);
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_port = (in_port_t) htons(port);
    recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(udpSocket, (struct sockaddr*) &recv_addr, sizeof recv_addr) < 0) {
        errno_abort("bind");
    }


    if((broadcastSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        errno_abort("socket");
    }

    if (setsockopt(broadcastSocket, SOL_SOCKET, SO_BROADCAST, &trueFlag, sizeof trueFlag) < 0) {
        errno_abort("setsockopt");
    }

    memset(&broadcastAddress, 0, sizeof broadcastAddress);
    broadcastAddress.sin_family = AF_INET;
    broadcastAddress.sin_port = (in_port_t) htons(port);
    // broadcasting address for unix (?)
    inet_aton("255.255.255.255", &broadcastAddress.sin_addr);

}

void UdpThread::genericBroadcast(UdpMessageCode code, const char *payload) const {
    char sbuf[HEADER_SIZE + MAX_SIZE_OF_PAYLOAD] = {};
    if( strlen(payload) > MAX_SIZE_OF_PAYLOAD ){
        std::cout << "The payload is too big!\n";
        return;
    }
    snprintf(sbuf, sizeof(sbuf), "%d;%s", code, payload);
    if(broadcastSocket == 0){
    	std::cout<<"!\n";
    }
    if (sendto(broadcastSocket, sbuf, strlen(sbuf) + 1, 0, (struct sockaddr *) &broadcastAddress, sizeof broadcastAddress) < 0) {
        errno_abort("send");
    }

}


void UdpThread::broadcastNewNode(){
    std::string buf;
    genericBroadcast(NEW_NODE_IN_NETWORK, buf.c_str());
}

void UdpThread::broadcastNewFile(const ResourceInfo& resource)
{
    char sbuf[MAX_SIZE_OF_PAYLOAD] = {};
    snprintf(sbuf, sizeof(sbuf),
             "%s;%lu;%d",
             resource.resourceName.c_str(),
             resource.revokeHash,
             resource.sizeInBytes);

    genericBroadcast(NEW_RESOURCE_AVAILABLE, sbuf);
}

void UdpThread::broadcastRevokeFile(const std::string& resource){
    char sbuf[MAX_SIZE_OF_PAYLOAD] = {};
    snprintf(sbuf, sizeof(sbuf), "%s", resource.c_str());
    genericBroadcast(OWNER_REVOKED_RESOURCE, sbuf);
}

void UdpThread::broadcastFileDeleted(const ResourceInfo& resource){
    char sbuf[MAX_SIZE_OF_PAYLOAD] = {};
    snprintf(sbuf, sizeof(sbuf), "%s", resource.resourceName.c_str());
    genericBroadcast(NODE_DELETED_RESOURCE, sbuf);
}

void UdpThread::broadcastLogout(){
//    std::stringstream ss;
//    char sbuf[MAX_SIZE_OF_PAYLOAD] = {};
//
//    for(const auto& resource: resources){
//        if(ss.str().size() + resource.resourceName.size() > MAX_SIZE_OF_PAYLOAD){ //todo chyba niepotrzebne to wysyłać
//            snprintf(sbuf, sizeof(sbuf), "%s", ss.str().c_str());
//            genericBroadcast(NODE_LEFT_NETWORK, sbuf);
//            ss.clear();
//        }
//        ss << ";" << resource.resourceName;
//    }
//    memset(sbuf, 0, sizeof sbuf);
//    snprintf(sbuf, sizeof(sbuf), "%s", ss.str().c_str());
    std::string buf;
    genericBroadcast(NODE_LEFT_NETWORK, buf.c_str());
}


//te funkcje handlujące nie tworzą nowych nitek deserializacja i aktualizacja struktur
void UdpThread::handleNewResourceAvailable(char *message, sockaddr_in sockaddr) {
    ResourceInfo resource = ResourceInfo::deserializeResource(message);
    sharedStructs.networkResourcesMutex.lock();
    sharedStructs.networkResources[convertAddress(sockaddr)][resource.resourceName] = resource;
    sharedStructs.networkResourcesMutex.unlock();

}

void UdpThread::handleOwnerRevokedResource(char *message, sockaddr_in sockaddr) {
    ResourceInfo resource = ResourceInfo::deserializeResource(message);
    sharedStructs.networkResourcesMutex.lock();
//    sharedStructs.networkResources[convertAddress(sockaddr)][resource.resourceName].isRevoked = true;
    sharedStructs.networkResources[convertAddress(sockaddr)].erase(resource.resourceName);
    sharedStructs.networkResourcesMutex.unlock();
}

void UdpThread::handleNodeDeletedResource(char *message, sockaddr_in sockaddr) {
    ResourceInfo resource = ResourceInfo::deserializeResource(message);
    sharedStructs.networkResourcesMutex.lock();
    sharedStructs.networkResources[convertAddress(sockaddr)].erase(resource.resourceName);
    sharedStructs.networkResourcesMutex.unlock();
}

void UdpThread::handleNewNodeInNetwork(sockaddr_in sockaddr) {
    sharedStructs.networkResourcesMutex.lock();
    sharedStructs.networkResources.insert(std::make_pair(convertAddress(sockaddr), std::map<std::string, ResourceInfo>()));
    sharedStructs.networkResourcesMutex.unlock();
    sendMyState(sockaddr);
}



void UdpThread::handleStateOfNode(char *message, sockaddr_in sockaddr) {
    std::vector<ResourceInfo> resources = ResourceInfo::deserializeVectorOfResources(message);
    sharedStructs.networkResourcesMutex.lock();
    if( resources.empty() ){
        sharedStructs.networkResources[convertAddress(sockaddr)] = std::map<std::string, ResourceInfo>();
    }
    for(const auto & r : resources){
        sharedStructs.networkResources[convertAddress(sockaddr)][r.resourceName] = r;
    }
    sharedStructs.networkResourcesMutex.unlock();
}

void UdpThread::handleNodeLeftNetwork(sockaddr_in sockaddr) {
    sharedStructs.networkResourcesMutex.lock();
    sharedStructs.networkResources.erase(convertAddress(sockaddr));
    sharedStructs.networkResourcesMutex.unlock();
}

void UdpThread::sendMyState(sockaddr_in newPeer) {
    std::stringstream ss;
    sharedStructs.localResourcesMutex.lock();
    char payload[MAX_SIZE_OF_PAYLOAD] = {};
    char sbuf[HEADER_SIZE + MAX_SIZE_OF_PAYLOAD] = {};
    for(const auto& [resourceName, resource] : sharedStructs.localResources){
        if(ss.str().size() + resourceName.size() + sizeof(long) + sizeof(int) > MAX_SIZE_OF_PAYLOAD){
            snprintf(payload, sizeof(payload), "%s", ss.str().c_str());
            memset(sbuf, 0 , sizeof(sbuf));
            snprintf(sbuf, sizeof(sbuf), "%d;%s", STATE_OF_NODE, payload);
            std::cout<<"sendmystateto: "<< inet_ntoa(newPeer.sin_addr) << " port: " << htons (newPeer.sin_port) << "\n";
            if (sendto(udpSocket, sbuf, strlen(sbuf) + 1, 0, (struct sockaddr *) &newPeer, sizeof newPeer) < 0) {
                errno_abort("send");
            }
            ss.clear();
        }
        ss << resource.resourceName  << ";"  << resource.revokeHash  << ";"  << resource.sizeInBytes  << ";";
    }

    sharedStructs.localResourcesMutex.unlock();
    memset(sbuf, 0 , sizeof(sbuf));
    memset(payload, 0 , sizeof(payload));
    snprintf(payload, sizeof(payload), "%s", ss.str().c_str());
    snprintf(sbuf, sizeof(sbuf), "%d;%s", STATE_OF_NODE, payload);
    std::cout<<"sendmystateto: "<< inet_ntoa(newPeer.sin_addr) << " port: " << htons (newPeer.sin_port) << "\n";
    if (sendto(udpSocket, sbuf, strlen(sbuf) + 1, 0, (struct sockaddr *) &newPeer, sizeof newPeer) < 0) {
        errno_abort("send");
    }

}

void UdpThread::setBarrier(pthread_barrier_t *ptr) {
	barrier = ptr;
}
