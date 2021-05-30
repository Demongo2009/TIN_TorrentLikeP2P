#ifndef TIN_TORRENTLIKEP2P_TORRENTCLIENT_H
#define TIN_TORRENTLIKEP2P_TORRENTCLIENT_H

#include <vector>
#include <mutex>
#include <map>
#include "ResourceInfo.h"
#include "PeerInfo.h"
#include "Message.h"

class TorrentClient {
private:
    // w sprawku napisalsimy ze bedzie mapa<nazwazasobu - metadane> i mozemy tak zrobić, na razie zostawiam tak
    std::vector<ResourceInfo> localResources; //mozliwe ze bedize trzeba inna strukture zamiast generyczną
    std::vector<ResourceInfo> networkResources;
    std::vector<PeerInfo> nodes;

    std::mutex localResourcesMutex;
    std::mutex networkResourcesMutex;
    std::mutex nodesMutex;

    void init();

    void* runTcpServerThread();
    void* runUdpServerThread();
    void* runCliThread();


//broadcast functions
    void genericBroadcast(UdpMessageCode code, char* payload);
    void broadcastNewNode();
    void broadcastNewFile(std::string fileName, std::string hash, int fileSize);
    void broadcastRevokeFile(std::string fileName);
    void broadcastFileDeleted(std::string fileName);
    void broadcastLogout(std::vector<std::string> fileList);

//functions handling broadcasted messages - UDP server
    void handleNewResourceAvailable(char *message);
    void handleOwnerRevokedResource(char *message);
    void handleNodeDeletedResource(char *message);
    void handleNewNodeInNetwork(char *message);
    void handleStateOfNode(char *message);
    void handleNodeLeftNetwork(char *message);
public:
    void run();

	void serverRecv();


};


#endif //TIN_TORRENTLIKEP2P_TORRENTCLIENT_H
