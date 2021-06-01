#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <string>
#include <sstream>

#include "../../include/threads/TorrentClient.h"


void TorrentClient::run() {
    std::thread serverUdpThread(&TorrentClient::runUdpServerThread, this);
    std::thread serverTcpThread(&TorrentClient::runTcpServerThread, this);
    std::thread cliThread(&TorrentClient::runCliThread, this);

    cliThread.join();
    /**
     * 1.init struktur
     * 2.pthread_create(runServerThread)
     * 3.sleep(1sec)? - zeby poczekac chwilkę żebyśmy zdazyli sfetchowac stan sieci, mozemy to tez w wątku klienta przy czym wysietlimy jakas informacje typu "sekunda... inicjlaizacja węzła"
     * 4.phread_creat(runCliThread)
     */
}

void TorrentClient::handleExit() {
    close(broadcastSocket);
}

void TorrentClient::signalHandler() {
    keepGoing = false;
    handleExit();
}

void TorrentClient::receive(int socket, bool tcp){
    char rbuf[MAX_MESSAGE_SIZE];
    memset(rbuf, 0, MAX_MESSAGE_SIZE);
    struct sockaddr_in clientAddr;
    socklen_t clientLength = sizeof(sockaddr_in);
    if (recvfrom(socket, rbuf, sizeof(rbuf) - 1, 0,(struct sockaddr *) &clientAddr, &clientLength) < 0) {
        perror("receive error");
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    printf("recv: %s\n", rbuf);
#endif
    char header[HEADER_SIZE];
    char payload[MAX_SIZE_OF_PAYLOAD];
    memset(header, 0, HEADER_SIZE);
    memset(payload, 0, MAX_SIZE_OF_PAYLOAD);
    snprintf(header, sizeof(header), "%s", rbuf);
    snprintf(payload, sizeof(payload), "%s", rbuf+HEADER_SIZE+1);
    if(tcp){
        handleTcpMessage(header, payload, clientAddr);
    }else{
        handleUdpMessage(header, payload, clientAddr);
    }

}

/**
 *
 * TCP BLOCK
 *
 */
void TorrentClient::initTcp(){
    struct sockaddr_in serverAddr;

    tcpSocket = socket(PF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;

    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(address.c_str());


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
}

int TorrentClient::acceptClient() {
    printf("waiting for client...\n");
    struct sockaddr_in clientAddr;
    socklen_t size = sizeof(clientAddr);
    int clientSocket = accept(tcpSocket, (struct sockaddr *) &clientAddr, &size);
    if (clientSocket == -1){
        printf("ACCEPT ERROR: %s\n", strerror(errno));
    }

    int currentReturnCode = getpeername(clientSocket, (struct sockaddr *) &clientAddr, &size);
    if (currentReturnCode == -1){
        printf("GETPEERNAME ERROR: %s\n", strerror(errno));
    }
    else {
        std::cout<<"Client address: "<< inet_ntoa(clientAddr.sin_addr);
    }
    connectedClients.emplace_back(clientSocket);
    return clientSocket;
}

[[noreturn]] void TorrentClient::runTcpServerThread() {

    initTcp();
    while (true) {
        receive(acceptClient(), true);
    }
}


void TorrentClient::handleTcpMessage(char *header, char *payload, sockaddr_in sockaddr) {
    switch (atoi(header)) {
        case DEMAND_CHUNK:
            handleDemandChunk(payload);
            break;
        case MY_STATE_BEFORE_FILE_SENDING:
            handleMyStateBeforeFileSending(payload);
            break;
        case CHUNK_TRANSFER:
            handleChunkTransfer(payload);
            break;
        case ERROR_AFTER_SYNCHRONIZATION:
            handleErrorAfterSynchronization(payload);
            break;
        case ERROR_WHILE_RECEIVING:
            handleErrorWhileReceiving(payload);
            break;
        case ERROR_WHILE_SENDING:
            handleErrorWhileSending(payload);
            break;
    }
}


// sending convention HEADER , {';' , MESSAGE_ELEMENT};

void TorrentClient::handleDemandChunk(char *payload) {

}

void TorrentClient::handleMyStateBeforeFileSending(char *payload) {

}

void TorrentClient::handleChunkTransfer(char *payload) {

}

void TorrentClient::handleErrorAfterSynchronization(char *payload) {

}

void TorrentClient::handleErrorWhileReceiving(char *payload) {

}

void TorrentClient::handleErrorWhileSending(char *payload) {

}


/**
 *
 * UDP BLOCK
 *
 */

void TorrentClient::handleUdpMessage(char *header, char *payload, sockaddr_in sockaddr) {
    switch (atoi(header)) {
        case NEW_RESOURCE_AVAILABLE:
            handleNewResourceAvailable(payload);
            break;
        case OWNER_REVOKED_RESOURCE:
            handleOwnerRevokedResource(payload);
            break;
        case NODE_DELETED_RESOURCE:
            handleNodeDeletedResource(payload);
            break;
        case NEW_NODE_IN_NETWORK:
            handleNewNodeInNetwork(payload, sockaddr);
            break;
        case STATE_OF_NODE:
            handleStateOfNode(payload);
            break;
        case NODE_LEFT_NETWORK:
            handleNodeLeftNetwork(payload);
            break;
    }
}

void errno_abort(const std::string& header){
    perror(header.c_str());
    exit(EXIT_FAILURE);
}

[[noreturn]] void TorrentClient::runUdpServerThread() {
    initUdp();
    broadcastNewNode();
    while (true){
        receive(udpSocket, false);
    }
}

void TorrentClient::initUdp() {
    struct sockaddr_in recv_addr;
    int trueFlag = 1;
    if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        errno_abort("socket");
    }
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
    inet_aton("127.255.255.255", &broadcastAddress.sin_addr);

}

void TorrentClient::genericBroadcast(UdpMessageCode code, char *payload) const {

    // dont know if addr will be needed
//	char sbuf[HEADER_SIZE+5] = {};
//	snprintf(sbuf, sizeof(sbuf), "%d;%d:%d", NEW_NODE, SERVERADDR, SERVERPORT);

    char sbuf[HEADER_SIZE + MAX_SIZE_OF_PAYLOAD] = {};
    snprintf(sbuf, sizeof(sbuf), "%d;%s", code, payload);

    if (sendto(broadcastSocket, sbuf, strlen(sbuf) + 1, 0, (struct sockaddr *) &broadcastAddress, sizeof broadcastAddress) < 0) {
        errno_abort("send");
    }

#ifdef DEBUG
    printf("send new node: %s\n", sbuf);
#endif

}


void TorrentClient::broadcastNewNode(){
    char* buf = {};
    genericBroadcast(NEW_NODE_IN_NETWORK, buf);
}

void TorrentClient::broadcastNewFile(const ResourceInfo& resource) {
    char sbuf[MAX_SIZE_OF_PAYLOAD] = {};
    snprintf(sbuf, sizeof(sbuf),
             "%s;%s;%d",
             resource.resourceName.c_str(),
             resource.revokeHash.c_str(),
             resource.sizeInBytes);
    genericBroadcast(NEW_RESOURCE_AVAILABLE, sbuf);
}

void TorrentClient::broadcastRevokeFile(const ResourceInfo& resource){
    char sbuf[MAX_SIZE_OF_PAYLOAD] = {};
    snprintf(sbuf, sizeof(sbuf), "%s", resource.resourceName.c_str());
    genericBroadcast(OWNER_REVOKED_RESOURCE, sbuf);
}

void TorrentClient::broadcastFileDeleted(const ResourceInfo& resource){
    char sbuf[MAX_SIZE_OF_PAYLOAD] = {};
    snprintf(sbuf, sizeof(sbuf), "%s", resource.resourceName.c_str());
    genericBroadcast(NODE_DELETED_RESOURCE, sbuf);
}

void TorrentClient::broadcastLogout(const std::vector<ResourceInfo>& resources){
    std::stringstream ss;
    for(const auto& resource: resources){
        ss << ";" << resource.resourceName;
    }
    char sbuf[MAX_SIZE_OF_PAYLOAD] = {};
    snprintf(sbuf, sizeof(sbuf), "%s", ss.str().c_str()); //todo tu chyba trzeba w pętli bo 512 może być za mało
    genericBroadcast(NODE_LEFT_NETWORK, sbuf);
}


//te funkcje handlujące nie tworzą nowych nitek deserializacja i aktualizacja struktur
void TorrentClient::handleNewResourceAvailable(char *message) {

}

void TorrentClient::handleOwnerRevokedResource(char *message) {

}

void TorrentClient::handleNodeDeletedResource(char *message) {

}

void TorrentClient::handleNewNodeInNetwork(char *message, sockaddr_in sockaddr) {
    nodes_.insert(std::make_pair(std::make_pair(sockaddr.sin_addr.s_addr, sockaddr.sin_port), PeerInfo(sockaddr)));
    sendMyState(sockaddr);
}



void TorrentClient::handleStateOfNode(char *message) {

}

void TorrentClient::handleNodeLeftNetwork(char *message) {

}

/**
 *
 * CLI BLOCK
 *
 */


void TorrentClient::runCliThread() {
    //KUBA SPARSUJ

    while(keepGoing){
        ClientCommand parsedCommand;
        std::string filepath, resourceName;
        switch (parsedCommand) {
            case ADD_NEW_RESOURCE:
                handleClientAddResource(filepath, resourceName);
                break;
            case LIST_AVAILABLE_RESOURCES:
                handleClientListResources();
                break;
            case FIND_RESOURCE:
                handleClientFindResource(resourceName);
                break;
            case DOWNLOAD_RESOURCE:
                handleDownloadResource(resourceName);
                break;
            case REVOKE_RESOURCE:
                handleRevokeResource(resourceName);
                break;
            case EXIT:
                handleExit();
                keepGoing = false;
                break;
        }
    }
}

//te nowe nitki robią
void TorrentClient::handleClientAddResource(const std::string& filepath, const std::string& resourceName) {

}

void TorrentClient::handleClientListResources() {

}

void TorrentClient::handleClientFindResource(const std::string& resourceName) {

}

void TorrentClient::handleDownloadResource(const std::string& resourceName) {

}

void TorrentClient::handleRevokeResource(const std::string& resourceName) {

}

void TorrentClient::sendMyState(sockaddr_in newPeer) {
    std::stringstream ss;
    for(const auto& resource: localResources){
        ss << ";" << resource.resourceName;
    }
    char payload[MAX_SIZE_OF_PAYLOAD] = {};
    snprintf(payload, sizeof(payload), "%s", ss.str().c_str()); //todo tu chyba trzeba w pętli bo 512 może być za mało


    char sbuf[HEADER_SIZE + MAX_SIZE_OF_PAYLOAD] = {};

    snprintf(sbuf, sizeof(sbuf), "%d;%s", STATE_OF_NODE, payload);

    if (sendto(udpSocket, sbuf, strlen(sbuf) + 1, 0, (struct sockaddr *) &newPeer, sizeof newPeer) < 0) {
        errno_abort("send");
    }
}










