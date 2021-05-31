#include <thread>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <unistd.h>
#include <sstream>
#include <iostream>

#include "../../include/threads/TorrentClient.h"
#include "../../include/structs/Message.h"

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

[[noreturn]] void TorrentClient::runUdpServerThread() {
    initUdp();
    while (true){
        serverRecv();
    }
}

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

    int clientSocket = accept(socketFileDescriptor, (struct sockaddr *) &clientAddr, &serverAddressLength);
    if (clientSocket == -1){
        printf("ACCEPT ERROR: %s\n", strerror(errno));
    }

    int clientAddressLength = sizeof(clientAddr);
    currentReturnCode = getpeername(clientSocket, (struct sockaddr *) &clientAddr, &clientAddressLength);
    if (currentReturnCode == -1){
        printf("GETPEERNAME ERROR: %s\n", strerror(errno));
    }
    else {
        std::cout<<"Client address: "<<clientAddr.sin_addr;
    }
    connectedClients.emplace_back(clientSocket);
    return clientSocket;
}

[[noreturn]] void TorrentClient::runTcpServerThread() {

    initTcp();
    while (true) {
        handleTcpClient(acceptClient());
    }
}

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


// sending convention HEADER , {';' , MESSAGE_ELEMENT};

void errno_abort(const char* header)
{
	perror(header);
	exit(EXIT_FAILURE);
}

void TorrentClient::genericBroadcast(UdpMessageCode code, char *payload) {
    struct sockaddr_in sendAddr, recvAddr;
    int trueFlag = 1;
    int fd;
    if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        errno_abort("socket");
    }

    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &trueFlag, sizeof trueFlag) < 0) {
        errno_abort("setsockopt");
    }

    memset(&sendAddr, 0, sizeof sendAddr);
    sendAddr.sin_family = AF_INET;
    sendAddr.sin_port = (in_port_t) htons(port);
    // broadcasting address for unix (?)
    inet_aton("127.255.255.255", &sendAddr.sin_addr);

    // dont know if addr will be needed
//	char sbuf[HEADER_SIZE+5] = {};
//	snprintf(sbuf, sizeof(sbuf), "%d;%d:%d", NEW_NODE, SERVERADDR, SERVERPORT);

    char sbuf[HEADER_SIZE + MAX_SIZE_OF_PAYLOAD] = {};
    snprintf(sbuf, sizeof(sbuf), "%d;%s", code, payload);

    if (sendto(fd, sbuf, strlen(sbuf) + 1, 0, (struct sockaddr *) &sendAddr, sizeof sendAddr) < 0) {
        errno_abort("send");
    }

#ifdef DEBUG
    printf("send new node: %s\n", sbuf);
#endif

    close(fd);
}

void TorrentClient::serverRecv(){

	char rbuf[MAX_MESSAGE_SIZE];
    memset(rbuf, 0, MAX_MESSAGE_SIZE);
	if (recv(udpSocket, rbuf, sizeof(rbuf) - 1, 0) < 0) {
        errno_abort("recv");
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

	//TODO: w tym switchu w niektorych węzłach(albo i wszystkich)
	// zamiast payloadu bedzie trzeba przekazywać dodatkowo informacje o jaki węzeł chodzi itp.
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
			handleNewNodeInNetwork(payload);
			break;
	    case STATE_OF_NODE:
	        handleStateOfNode(payload);
	        break;
		case NODE_LEFT_NETWORK:
			handleNodeLeftNetwork(payload);
			break;
	}
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
	snprintf(sbuf, sizeof(sbuf), "%s", ss.str().c_str());
    genericBroadcast(NODE_LEFT_NETWORK, sbuf);
}


//te funkcje handlujące nie tworzą nowych nitek deserializacja i aktualizacja struktur
void TorrentClient::handleNewResourceAvailable(char *message) {}
void TorrentClient::handleOwnerRevokedResource(char *message) {}
void TorrentClient::handleNodeDeletedResource(char *message) {}
void TorrentClient::handleNewNodeInNetwork(char *message) {}
void TorrentClient::handleStateOfNode(char *message) {}
void TorrentClient::handleNodeLeftNetwork(char *message) {}


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

void TorrentClient::handleExit() {

}

void TorrentClient::signalHandler() {
    keepGoing = false;
    handleExit();
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

}

void TorrentClient::handleTcpClient(int clientSocket) {
    char rbuf[MAX_MESSAGE_SIZE];
    memset(rbuf, 0, MAX_MESSAGE_SIZE);
    if (recv(tcpSocket, rbuf, sizeof(rbuf) - 1, 0) < 0) {
        errno_abort("recv");
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

    //TODO: w tym switchu w niektorych węzłach(albo i wszystkich)
    // zamiast payloadu bedzie trzeba przekazywać dodatkowo informacje o jaki węzeł chodzi itp.

//    MY_STATE_BEFORE_FILE_SENDING=141,   // tablica krotek: (resourceName, revokeHash, sizeInBytes)
//    CHUNK_TRANSFER=142,                 // indexOfChunk, offsetFromChunkStart, data
//    ERROR_AFTER_SYNCHRONIZATION=440,    // EMPTY
//    ERROR_WHILE_SENDING=540,            // EMPTY
//    ERROR_WHILE_RECEIVING=541,

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




