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
  * inicjalizacja struktur
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

///////////////////////// demongos edit

#define SERVERPORT 5555
#define HEADER_SIZE 3
#define MAX_RECV_SIZE 256
#define MAX_MESSAGE_SIZE 253
#define SERVERADDR 1981001006

// sending convention HEADER , {';' , MESSAGE_ELEMENT};

void errno_abort(const char* header)
{
	perror(header);
	exit(EXIT_FAILURE);
}

void TorrentClient::genericBroadcast(UdpMessageCode code, char *payload) {
    struct sockaddr_in sendAddr, recvAddr;
    int trueflag = 1;
    int fd;
    if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        errno_abort("socket");
    }

    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &trueflag, sizeof trueflag) < 0) {
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

    char sbuf[HEADER_SIZE + MAX_MESSAGE_SIZE] = {};
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
	int trueflag = 1;
	int fd;
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        errno_abort("socket");
    }
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &trueflag, sizeof trueflag) < 0) {
        errno_abort("setsockopt");
    }

	memset(&recv_addr, 0, sizeof recv_addr);
	recv_addr.sin_family = AF_INET;
	recv_addr.sin_port = (in_port_t) htons(SERVERPORT);
	recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(fd, (struct sockaddr*) &recv_addr, sizeof recv_addr) < 0) {
        errno_abort("bind");
    }

	char rbuf[MAX_RECV_SIZE] = {};
	if (recv(fd, rbuf, sizeof(rbuf)-1, 0) < 0) {
        errno_abort("recv");
    }
	
#ifdef DEBUG
	printf("recv: %s\n", rbuf);
#endif
	close(fd);

	char header[HEADER_SIZE] = {};
	char message[MAX_MESSAGE_SIZE] = {};

	snprintf(header, sizeof(header), "%s", rbuf);
	snprintf(message, sizeof(message), "%s", rbuf+HEADER_SIZE+1);

	switch (atoi(header)) {
		case NEW_RESOURCE_AVAILABLE:
			handleNewResourceAvailable(message);
			break;
		case OWNER_REVOKED_RESOURCE:
            handleOwnerRevokedResource(message);
			break;
		case NODE_DELETED_RESOURCE:
			handleNodeDeletedResource(message);
			break;
		case NEW_NODE_IN_NETWORK:
			handleNewNodeInNetwork(message);
			break;
	    case STATE_OF_NODE:
	        handleStateOfNode(message);
	        //TODO: to dostajemy od każdego jak wyślemy broadcast ze jestesmy nowi w sieci
	        break;
		case NODE_LEFT_NETWORK:
			handleNodeLeftNetwork(message);
			break;
	}
}

void TorrentClient::broadcastNewNode(){
    genericBroadcast(NEW_NODE_IN_NETWORK, "");
}

void TorrentClient::broadcastNewFile(std::string fileName, std::string hash, int fileSize) {
    char sbuf[HEADER_SIZE + MAX_MESSAGE_SIZE] = {};
    snprintf(sbuf, sizeof(sbuf), "%d;%s;%s;%d", NEW_RESOURCE_AVAILABLE, fileName.c_str(), hash.c_str(), fileSize);
    genericBroadcast(NEW_RESOURCE_AVAILABLE, sbuf);
}

void TorrentClient::broadcastRevokeFile(std::string fileName){
	char sbuf[HEADER_SIZE+MAX_MESSAGE_SIZE] = {};
	snprintf(sbuf, sizeof(sbuf), "%d;%s", OWNER_REVOKED_RESOURCE, fileName.c_str());
    genericBroadcast(OWNER_REVOKED_RESOURCE, sbuf);
}

void TorrentClient::broadcastFileDeleted(std::string fileName){
	char sbuf[HEADER_SIZE+MAX_MESSAGE_SIZE] = {};
	snprintf(sbuf, sizeof(sbuf), "%d;%s", NODE_DELETED_RESOURCE, fileName.c_str());
    genericBroadcast(NODE_DELETED_RESOURCE, sbuf);
}

void TorrentClient::broadcastLogout(std::vector<std::string> fileList){
	std::stringstream ss;
	for(const auto& f: fileList){
		ss << ";" << f;
	}
	char sbuf[HEADER_SIZE] = {};
	snprintf(sbuf, sizeof(sbuf), "%d%s", NODE_LEFT_NETWORK, ss.str().c_str());
    genericBroadcast(NODE_LEFT_NETWORK, sbuf);
}



void TorrentClient::handleNewResourceAvailable(char *message) {}
void TorrentClient::handleOwnerRevokedResource(char *message) {}
void TorrentClient::handleNodeDeletedResource(char *message) {}
void TorrentClient::handleNewNodeInNetwork(char *message) {}
void TorrentClient::handleStateOfNode(char *message) {}
void TorrentClient::handleNodeLeftNetwork(char *message) {}


