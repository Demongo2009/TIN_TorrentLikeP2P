#ifndef TIN_TORRENTLIKEP2P_TCPTHREAD_H
#define TIN_TORRENTLIKEP2P_TCPTHREAD_H

#include <vector>
#include <mutex>
#include <map>
#include <sys/socket.h>

#include "../structs/Message.h"
#include "../structs/ResourceInfo.h"
#include "../structs/ConnectedPeerInfo.h"
#include "../structs/SharedStructs.h"

class TcpThread{

public:
    explicit TcpThread(SharedStructs& structs) : sharedStructs(structs){ keepGoing = true; }

    static void start(TcpThread* tcpObj){
    	tcpObj->runTcpServerThread();
    }
    void terminate();
    void initTcp();
    void runTcpServerThread();

    bool receiveSync(int socket, std::optional<struct sockaddr_in> sockaddr);
    void sendSync(int socket);


	void setBarrier(pthread_barrier_t *ptr);
    bool receiveHeader(int socket, TcpMessageCode code);
    static void sendHeader(int socket, TcpMessageCode code);
private:
    bool keepGoing;
    SharedStructs& sharedStructs;
    int tcpSocket;
    const int port = 5555;
    std::string myAddress;
    std::map<int, ConnectedPeerInfo> connectedClients;
	pthread_barrier_t* barrier;


    int acceptClient();
    void receive(int socket);

    void handleTcpMessage(char *header, char *payload, int socket);
    void demandChunkJob(char *payload, int sockaddr);
    void sendChunks(const DemandChunkMessage &message, int socket);
    void clearPeerInfo(struct sockaddr_in sockaddr);
    bool validateChunkDemand(const DemandChunkMessage& message);

    static void printTcpThreadMessage(std::string);
};

#endif //TIN_TORRENTLIKEP2P_TCPTHREAD_H
