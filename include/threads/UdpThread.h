//
// Created by bartlomiej on 03.06.2021.
//

#ifndef TIN_TORRENTLIKEP2P_UDPTHREAD_H
#define TIN_TORRENTLIKEP2P_UDPTHREAD_H

#include <vector>
#include <mutex>
#include <map>
#include <sys/socket.h>

#include "../structs/Message.h"
#include "../structs/ResourceInfo.h"
#include "../structs/ConnectedPeerInfo.h"
#include "../structs/SharedStructs.h"

class UdpThread{
public:
    UdpThread(SharedStructs& structs) : sharedStructs(structs), keepGoing(true){}

	static void start(UdpThread* udpObj){
    	udpObj->runUdpServerThread();
    }
    void runUdpServerThread();

    void broadcastNewFile(const ResourceInfo& resource);

    void broadcastRevokeFile(const std::string& resource);
    void terminate();

	void setBarrier(pthread_barrier_t *ptr);
	void initUdp();

private:
    SharedStructs& sharedStructs;
    int udpSocket;
    const int port = 5555;
    std::string myAddress;
    struct sockaddr_in recv_addr;
    bool keepGoing;
    struct sockaddr_in broadcastAddress;
    int broadcastSocket;
    pthread_barrier_t* barrier;

    void receive();
    //broadcast functions
    void genericBroadcast(UdpMessageCode code, const char* payload) const;
    void broadcastNewNode();

    void broadcastFileDeleted(const ResourceInfo& resource);
    void broadcastLogout();

//functions handling broadcasted messages - UDP server
    void handleNewResourceAvailable(char *message, sockaddr_in sockaddr);
    void handleOwnerRevokedResource(char *message, sockaddr_in sockaddr);
    void handleNodeDeletedResource(char *message, sockaddr_in sockaddr);

    void handleStateOfNode(char *message, sockaddr_in sockaddr);
    void handleNodeLeftNetwork(sockaddr_in sockaddr);
    void handleUdpMessage(char *header, char *payload, sockaddr_in sockaddr);

    void handleNewNodeInNetwork(sockaddr_in sockaddr);

    void sendMyState(sockaddr_in in);
};

#endif //TIN_TORRENTLIKEP2P_UDPTHREAD_H
