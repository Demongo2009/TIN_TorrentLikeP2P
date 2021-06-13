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
#include "../../include/utils/utils.h"
#include "../../include/threads/UdpThread.h"

const char* LOCALHOST_ADDRESS = "127.0.0.1";
const char* BROADCAST_ADDRESS = "255.255.255.255";

void UdpThread::handleUdpMessage(char *header, char *payload, sockaddr_in sockaddr) {
    switch (std::stoi(header)) {
        case NEW_RESOURCE_AVAILABLE:
            handleNewResourceAvailable(payload, sockaddr);
            break;
        case OWNER_REVOKED_RESOURCE:
            handleOwnerRevokedResource(payload);
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
        default:
            throw std::runtime_error("Unhandled UDP message!");
    }
}

void UdpThread::runUdpServerThread() {
    initUdp();

	pthread_barrier_wait(barrier);
    broadcastNewNode();
    while (keepGoing){
        try {
            receive();
        }catch(std::exception& e ){
            std::cout<<"UDP catch: "<<e.what()<<std::endl;
        }
    }
}

void UdpThread::terminate(){
    keepGoing = false;
    broadcastLogout();
    close(udpSocket);
    exit(0);
}

void UdpThread::receive(){
    char receiveBuf[MAX_MESSAGE_SIZE];
    memset(receiveBuf, 0, MAX_MESSAGE_SIZE);
    struct sockaddr_in clientAddr{};

    socklen_t clientLength = sizeof(sockaddr_in);
    if (recvfrom(udpSocket, receiveBuf, sizeof(receiveBuf) - 1, 0,(struct sockaddr *) &clientAddr, &clientLength) < 0) {
        perror("receive error");
        exit(EXIT_FAILURE);
    }
    clientAddr.sin_port = (in_port_t) htons(port);

    printUdpThreadMessage(std::string("Received: ") + receiveBuf);


    char header[HEADER_SIZE];
    char payload[MAX_SIZE_OF_PAYLOAD];
    memset(header, 0, HEADER_SIZE);
    memset(payload, 0, MAX_SIZE_OF_PAYLOAD);

    snprintf(header, HEADER_SIZE, "%s", receiveBuf);
    snprintf(payload, MAX_SIZE_OF_PAYLOAD, "%s", receiveBuf+HEADER_SIZE);
    std::string clientAddressString = inet_ntoa(clientAddr.sin_addr);
    if(clientAddressString != myAddress && clientAddressString != LOCALHOST_ADDRESS){
        handleUdpMessage(header, payload, clientAddr);
    }

}

void UdpThread::initUdp() {
    int trueFlag = 1;
    if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        errno_abort("socket initUdp()");
    }
    myAddress = getMyAddress(udpSocket);

    if (setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof trueFlag) < 0) {
        errno_abort("setsockopt initUdp()");
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

    inet_aton(BROADCAST_ADDRESS, &broadcastAddress.sin_addr);

}

void UdpThread::genericBroadcast(UdpMessageCode code, const char *payload) const {
    char sbuf[HEADER_SIZE + MAX_SIZE_OF_PAYLOAD] = {};
    if( strlen(payload) > MAX_SIZE_OF_PAYLOAD ){
        std::cout << "The payload is too big!\n";
        return;
    }
    snprintf(sbuf, sizeof(sbuf), "%d;%s", code, payload);
    if (sendto(broadcastSocket, sbuf, strlen(sbuf) + 1, 0, (struct sockaddr *) &broadcastAddress, sizeof broadcastAddress) < 0) {
        errno_abort("send genericBroadcast()");
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
             "%s;%lu;%llu",
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
    std::string buf;
    genericBroadcast(NODE_LEFT_NETWORK, buf.c_str());
}


void UdpThread::handleNewResourceAvailable(char *message, sockaddr_in sockaddr) {
    ResourceInfo resource = ResourceInfo::deserializeResource(message);
    sharedStructs.addNetworkResource(sockaddr, resource);
}

void UdpThread::handleOwnerRevokedResource(char *message) {
    ResourceInfo resource = ResourceInfo::deserializeResource(message);
    sharedStructs.deleteNetworkResource(resource.resourceName);
    //TODO: a co z lokalnym plikiem, czy dalej powinien wyswietlac sie na liscie?
}

void UdpThread::handleNodeDeletedResource(char *message, sockaddr_in sockaddr) {
    ResourceInfo resource = ResourceInfo::deserializeResource(message);
    sharedStructs.deleteResourceFromNode(sockaddr, resource);
}

void UdpThread::handleNewNodeInNetwork(sockaddr_in sockaddr) {
    sharedStructs.addNetworkNode(sockaddr);
    sendMyState(sockaddr);
}

void UdpThread::handleStateOfNode(char *message, sockaddr_in sockaddr) {
    std::vector<ResourceInfo> resources = ResourceInfo::deserializeVectorOfResources(message);
    sharedStructs.registerNewNodeWithItsResources(sockaddr, resources);
}

void UdpThread::handleNodeLeftNetwork(sockaddr_in sockaddr) {
    sharedStructs.deleteNetworkNode(sockaddr);
}

void UdpThread::sendMyState(sockaddr_in newPeer) {
    char payload[MAX_SIZE_OF_PAYLOAD] = {};
    char sbuf[HEADER_SIZE + MAX_SIZE_OF_PAYLOAD] = {};

    std::vector<std::string> messagePayloads = sharedStructs.getLocalStateString();

    for(const auto& payloadStr: messagePayloads){
        memset(sbuf, 0 , sizeof(sbuf));
        memset(payload, 0 , sizeof(payload));
        snprintf(payload, sizeof(payload), "%s", payloadStr.c_str());
        snprintf(sbuf, sizeof(sbuf), "%d;%s", STATE_OF_NODE, payload);
        if (sendto(udpSocket, sbuf, strlen(sbuf) + 1, 0, (struct sockaddr *) &newPeer, sizeof newPeer) < 0) {
            errno_abort("send");
        }
    }
}

void UdpThread::setBarrier(pthread_barrier_t *ptr) {
	barrier = ptr;
}

void UdpThread::printUdpThreadMessage(const std::string& message) {
    std::cout<<"\tUDP THREAD:"<<message<<std::endl;
}
