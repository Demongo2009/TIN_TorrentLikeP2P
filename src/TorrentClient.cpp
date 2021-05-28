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

void *TorrentClient::runServerThread() {
    /**
     * na początku: rozglos ze jestes nowy
     *
     * while(true){
     *  tutaj typowy schemat pracy serwera(z uxpow mozna przekopiowac), czyli:
     *      1. nasłuchuj na połączenie/jakiś komunikat
     *      2. zobacz jaki to komunikat
     *      3. wywolaj odpowiednią funkcję w zależności od rodzaju komunikatu
     *      4. wróc do nasłuchiwania
     * }
     * */

    return nullptr;
}

///////////////////////// demongos edit

#define SERVERPORT 5555
#define HEADER_SIZE 3
#define MAX_RECV_SIZE 256
#define MAX_MESSAGE_SIZE 253
#define SERVERADDR 1981001006

// sending convention HEADER , {';' , MESSAGE_ELEMENT};

typedef enum BroadcastMessageType{
	ADD_NEW_FILE = 100,
	REVOKE_FILE = 110,
	NO_LONGER_HAVE_FILE = 111,
	NEW_NODE = 120,
	NODE_LOGOUT = 130
}BroadcastMessageType;

void errno_abort(const char* header)
{
	perror(header);
	exit(EXIT_FAILURE);
}

void TorrentClient::ServerRecv(){
	
	struct sockaddr_in recv_addr;
	int trueflag = 1;
	int fd;
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		errno_abort("socket");
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
				   &trueflag, sizeof trueflag) < 0)
		errno_abort("setsockopt");

	memset(&recv_addr, 0, sizeof recv_addr);
	recv_addr.sin_family = AF_INET;
	recv_addr.sin_port = (in_port_t) htons(SERVERPORT);
	recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(fd, (struct sockaddr*) &recv_addr, sizeof recv_addr) < 0)
		errno_abort("bind");

	char rbuf[MAX_RECV_SIZE] = {};
	if (recv(fd, rbuf, sizeof(rbuf)-1, 0) < 0)
		errno_abort("recv");
	
#ifdef DEBUG
	printf("recv: %s\n", rbuf);
#endif
	close(fd);

	char header[HEADER_SIZE] = {};
	char message[MAX_MESSAGE_SIZE] = {};

	snprintf(header, sizeof(header), "%s", rbuf);
	snprintf(message, sizeof(message), "%s", rbuf+HEADER_SIZE+1);

	switch (atoi(header)) {
		case ADD_NEW_FILE:
			addNewFile(message);
			break;
		case REVOKE_FILE:
			revokeFile(message);
			break;
		case NO_LONGER_HAVE_FILE:
			nodeDeletedFile(message);
			break;
		case NEW_NODE:
			addNewNode(message);
			break;
		case NODE_LOGOUT:
			nodeHaveBeenLogout(message);
			break;
	}
}

void TorrentClient::BroadcastNewNode(){
	struct sockaddr_in sendAddr, recvAddr;
	int trueflag = 1;
	int fd;
	if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		errno_abort("socket");

	if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST,
				   &trueflag, sizeof trueflag) < 0)
		errno_abort("setsockopt");

	memset(&sendAddr, 0, sizeof sendAddr);
	sendAddr.sin_family = AF_INET;
	sendAddr.sin_port = (in_port_t) htons(SERVERPORT);
	// broadcasting address for unix (?)
	inet_aton("127.255.255.255", &sendAddr.sin_addr);

	// dont know if addr will be needed
//	char sbuf[HEADER_SIZE+5] = {};
//	snprintf(sbuf, sizeof(sbuf), "%d;%d:%d", NEW_NODE, SERVERADDR, SERVERPORT);

	char sbuf[HEADER_SIZE] = {};
	snprintf(sbuf, sizeof(sbuf), "%d", NEW_NODE);

	if (sendto(fd, sbuf, strlen(sbuf) + 1, 0,
			   (struct sockaddr *) &sendAddr, sizeof sendAddr) < 0)
		errno_abort("send");

#ifdef DEBUG
	printf("send new node: %s\n", sbuf);
#endif

	close(fd);
}

void TorrentClient::BroadcastNewFile(std::string fileName, std::string hash, int fileSize){
	struct sockaddr_in sendAddr, recvAddr;
	int trueflag = 1;
	int fd;
	if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		errno_abort("socket");

	if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST,
				   &trueflag, sizeof trueflag) < 0)
		errno_abort("setsockopt");

	memset(&sendAddr, 0, sizeof sendAddr);
	sendAddr.sin_family = AF_INET;
	sendAddr.sin_port = (in_port_t) htons(SERVERPORT);
	// broadcasting address for unix (?)
	inet_aton("127.255.255.255", &sendAddr.sin_addr);


	char sbuf[HEADER_SIZE+MAX_MESSAGE_SIZE] = {};
	snprintf(sbuf, sizeof(sbuf), "%d;%s;%s;%d", ADD_NEW_FILE, fileName.c_str(), hash.c_str(), fileSize);
	if (sendto(fd, sbuf, strlen(sbuf) + 1, 0,
			   (struct sockaddr *) &sendAddr, sizeof sendAddr) < 0)
		errno_abort("send");

#ifdef DEBUG
	printf("send new file: %s\n", sbuf);
#endif

	close(fd);
}

void TorrentClient::BroadcastRevokeFile(std::string fileName){
	struct sockaddr_in sendAddr, recvAddr;
	int trueflag = 1;
	int fd;
	if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		errno_abort("socket");

	if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST,
				   &trueflag, sizeof trueflag) < 0)
		errno_abort("setsockopt");

	memset(&sendAddr, 0, sizeof sendAddr);
	sendAddr.sin_family = AF_INET;
	sendAddr.sin_port = (in_port_t) htons(SERVERPORT);
	// broadcasting address for unix (?)
	inet_aton("127.255.255.255", &sendAddr.sin_addr);

	char sbuf[HEADER_SIZE+MAX_MESSAGE_SIZE] = {};
	snprintf(sbuf, sizeof(sbuf), "%d;%s", REVOKE_FILE, fileName.c_str());

	if (sendto(fd, sbuf, strlen(sbuf) + 1, 0,
			   (struct sockaddr *) &sendAddr, sizeof sendAddr) < 0)
		errno_abort("send");

#ifdef DEBUG
	printf("send revoke: %s\n", sbuf);
#endif

	close(fd);
}

void TorrentClient::BroadcastFileDeleted(std::string fileName){
	struct sockaddr_in sendAddr, recvAddr;
	int trueflag = 1;
	int fd;
	if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		errno_abort("socket");

	if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST,
				   &trueflag, sizeof trueflag) < 0)
		errno_abort("setsockopt");

	memset(&sendAddr, 0, sizeof sendAddr);
	sendAddr.sin_family = AF_INET;
	sendAddr.sin_port = (in_port_t) htons(SERVERPORT);
	// broadcasting address for unix (?)
	inet_aton("127.255.255.255", &sendAddr.sin_addr);

	char sbuf[HEADER_SIZE+MAX_MESSAGE_SIZE] = {};
	snprintf(sbuf, sizeof(sbuf), "%d;%s", NO_LONGER_HAVE_FILE, fileName.c_str());

	if (sendto(fd, sbuf, strlen(sbuf) + 1, 0,
			   (struct sockaddr *) &sendAddr, sizeof sendAddr) < 0)
		errno_abort("send");

#ifdef DEBUG
	printf("send file deleted: %s\n", sbuf);
#endif

	close(fd);
}

void TorrentClient::BroadcastLogout(std::vector<std::string> fileList){
	struct sockaddr_in sendAddr, recvAddr;
	int trueflag = 1;
	int fd;
	if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		errno_abort("socket");

	if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST,
				   &trueflag, sizeof trueflag) < 0)
		errno_abort("setsockopt");

	memset(&sendAddr, 0, sizeof sendAddr);
	sendAddr.sin_family = AF_INET;
	sendAddr.sin_port = (in_port_t) htons(SERVERPORT);
	// broadcasting address for unix (?)
	inet_aton("127.255.255.255", &sendAddr.sin_addr);

	std::stringstream ss;
	for(const auto& f: fileList){
		ss << ";" << f;
	}

	char sbuf[HEADER_SIZE] = {};
	snprintf(sbuf, sizeof(sbuf), "%d%s", NODE_LOGOUT, ss.str().c_str());

	if (sendto(fd, sbuf, strlen(sbuf) + 1, 0,
			   (struct sockaddr *) &sendAddr, sizeof sendAddr) < 0)
		errno_abort("send");

#ifdef DEBUG
	printf("send logout: %s\n", sbuf);
#endif

	close(fd);
}

///////////////////////////

void *TorrentClient::runCliThread() {
    /**
     * 1. wypisz ze "trwa inicjalizacja" -> sleep(2s)
     * while(true){
     *  tutaj tez typowa pętla interfejsu, czyli:
     *      1. pobierz polecenie od uzytkownika
     *      2. oddeleguj odpowiedni wątek
     *      3. wróc do pobierania poleceń
     * }
     * */
    return nullptr;
}

///// TODO:

void TorrentClient::addNewFile(char *message) {

}

void TorrentClient::revokeFile(char *message) {

}

void TorrentClient::nodeDeletedFile(char *message) {

}

void TorrentClient::addNewNode(char *message) {

}

void TorrentClient::nodeHaveBeenLogout(char *message) {

}


