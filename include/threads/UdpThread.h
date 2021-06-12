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
    explicit UdpThread(SharedStructs& structs) : sharedStructs(structs), keepGoing(true){}

    void initUdp();
    void terminate();
    void setBarrier(pthread_barrier_t *ptr);
	static void start(UdpThread* udpObj){
    	udpObj->runUdpServerThread();
    }
    void runUdpServerThread();

    void broadcastNewFile(const ResourceInfo& resource);
    void broadcastRevokeFile(const std::string& resource);


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

    void genericBroadcast(UdpMessageCode code, const char* payload) const;
    void broadcastNewNode();
    void broadcastFileDeleted(const ResourceInfo& resource);
    void broadcastLogout();

    void handleNewResourceAvailable(char *message, sockaddr_in sockaddr);
    void handleOwnerRevokedResource(char *message);
    void handleNodeDeletedResource(char *message, sockaddr_in sockaddr);
    void handleStateOfNode(char *message, sockaddr_in sockaddr);
    void handleNodeLeftNetwork(sockaddr_in sockaddr);
    void handleUdpMessage(char *header, char *payload, sockaddr_in sockaddr);
    void handleNewNodeInNetwork(sockaddr_in sockaddr);

    void sendMyState(sockaddr_in in);

    static void printUdpThreadMessage(const std::string&);
};

#endif //TIN_TORRENTLIKEP2P_UDPTHREAD_H
