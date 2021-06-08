//
// Created by bartlomiej on 03.06.2021.
//

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
    TcpThread(SharedStructs& structs) : sharedStructs(structs){ keepGoing = true; }

    static void start(TcpThread* tcpObj){
    	tcpObj->runTcpServerThread();
    }
    void runTcpServerThread();

    bool receiveSync(int socket);

    void sendSync(int socket);
    void terminate();

	void setBarrier(pthread_barrier_t *ptr);
	void initTcp();

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

    static void sendHeader(int socket, TcpMessageCode code);

    void clearPeerInfo(int socket);
    bool validateChunkDemand(const DemandChunkMessage& message);

};

#endif //TIN_TORRENTLIKEP2P_TCPTHREAD_H
