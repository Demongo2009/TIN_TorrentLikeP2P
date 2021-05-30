#include <thread>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <unistd.h>
#include <sstream>

#include "../include/TorrentClient.h"
#include "../include/Message.h"

void TorrentClient::run() {
    /**
     * 1.init struktur
     * 2.pthread_create(runServerThread)
     * 3.sleep(1sec)? - zeby poczekac chwilkę żebyśmy zdazyli sfetchowac stan sieci, mozemy to tez w wątku klienta przy czym wysietlimy jakas informacje typu "sekunda... inicjlaizacja węzła"
     * 4.phread_creat(runCliThread)
     */
}

void TorrentClient::init() {
 /**
  * inicjalizacja wątków i struktur
  * */
}

void *TorrentClient::runUdpServerThread() {
    return nullptr;
}

void *TorrentClient::runTcpServerThread() {
    return nullptr;
}

void *TorrentClient::runCliThread() {
    return nullptr;
}

#define SERVERPORT 5555
#define SERVERADDR 1981001006

#define HEADER_SIZE 3
#define MAX_MESSAGE_SIZE HEADER_SIZE+MAX_SIZE_OF_PAYLOAD


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
    sendAddr.sin_port = (in_port_t) htons(SERVERPORT);
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
	
	struct sockaddr_in recv_addr;
	int trueFlag = 1;
	int fd;
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        errno_abort("socket");
    }
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof trueFlag) < 0) {
        errno_abort("setsockopt");
    }

	memset(&recv_addr, 0, sizeof recv_addr);
	recv_addr.sin_family = AF_INET;
	recv_addr.sin_port = (in_port_t) htons(SERVERPORT);
	recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(fd, (struct sockaddr*) &recv_addr, sizeof recv_addr) < 0) {
        errno_abort("bind");
    }

	char rbuf[MAX_MESSAGE_SIZE] = {};
	if (recv(fd, rbuf, sizeof(rbuf)-1, 0) < 0) {
        errno_abort("recv");
    }
	
#ifdef DEBUG
	printf("recv: %s\n", rbuf);
#endif
	close(fd);

	char header[HEADER_SIZE] = {};
	char payload[MAX_SIZE_OF_PAYLOAD] = {};

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
    genericBroadcast(NEW_NODE_IN_NETWORK, "");
}

void TorrentClient::broadcastNewFile(ResourceInfo resource) {
    char sbuf[MAX_SIZE_OF_PAYLOAD] = {};
    snprintf(sbuf, sizeof(sbuf),
             "%s;%s;%d",
             resource.resourceName.c_str(),
             resource.revokeHash.c_str(),
             resource.sizeInBytes);
    genericBroadcast(NEW_RESOURCE_AVAILABLE, sbuf);
}

void TorrentClient::broadcastRevokeFile(ResourceInfo resource){
	char sbuf[MAX_SIZE_OF_PAYLOAD] = {};
	snprintf(sbuf, sizeof(sbuf), "%s", resource.resourceName.c_str());
    genericBroadcast(OWNER_REVOKED_RESOURCE, sbuf);
}

void TorrentClient::broadcastFileDeleted(ResourceInfo resource){
	char sbuf[MAX_SIZE_OF_PAYLOAD] = {};
	snprintf(sbuf, sizeof(sbuf), "%s", resource.resourceName.c_str());
    genericBroadcast(NODE_DELETED_RESOURCE, sbuf);
}

void TorrentClient::broadcastLogout(std::vector<ResourceInfo> resources){
	std::stringstream ss;
	for(const auto& resource: resources){
		ss << ";" << resource.resourceName;
	}
	char sbuf[MAX_SIZE_OF_PAYLOAD] = {};
	snprintf(sbuf, sizeof(sbuf), "%s", ss.str().c_str());
    genericBroadcast(NODE_LEFT_NETWORK, sbuf);
}



void TorrentClient::handleNewResourceAvailable(char *message) {}
void TorrentClient::handleOwnerRevokedResource(char *message) {}
void TorrentClient::handleNodeDeletedResource(char *message) {}
void TorrentClient::handleNewNodeInNetwork(char *message) {}
void TorrentClient::handleStateOfNode(char *message) {}
void TorrentClient::handleNodeLeftNetwork(char *message) {}


