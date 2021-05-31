#ifndef TIN_TORRENTLIKEP2P_TORRENTCLIENT_H
#define TIN_TORRENTLIKEP2P_TORRENTCLIENT_H

#include <vector>
#include <mutex>
#include <map>

#include "../structs/Message.h"
#include "../structs/ResourceInfo.h"
#include "../structs/PeerInfo.h"

#define SERVERPORT 5555
#define SERVERADDR 1981001006

class TorrentClient {
public:
    TorrentClient(){
        keepGoing = true;
    }

    void run();

    void serverRecv();//ta funkcja to chyba w UdpServer będzie

    void signalHandler();
private:
    // w sprawku napisalsimy ze bedzie mapa<nazwazasobu - metadane> i mozemy tak zrobić, na razie zostawiam tak
    std::vector<ResourceInfo> localResources; //mozliwe ze bedize trzeba inna strukture zamiast generyczną
    std::vector<ResourceInfo> networkResources;
    std::vector<PeerInfo> nodes;
    std::vector<int> connectedClients;

    std::mutex localResourcesMutex;
    std::mutex networkResourcesMutex;
    std::mutex nodesMutex;
    bool keepGoing;
    int udpSocket;
    int tcpSocket;


    const int port = 5555;
    const std::string address = "127.0.0.1";
    void initUdp();
    void initTcp();
    int acceptClient();

    [[noreturn]] void runTcpServerThread();

    [[noreturn]] void runUdpServerThread();
    void runCliThread();

    void handleClientAddResource(const std::string& basicString, const std::string& basicString1);

    void handleClientListResources();

    void handleClientFindResource(const std::string& basicString);

    void handleDownloadResource(const std::string& basicString);

    void handleRevokeResource(const std::string& basicString);

    void handleExit();

    void handleTcpClient(int clientSocket);

//broadcast functions
    void genericBroadcast(UdpMessageCode code, char* payload);
    void broadcastNewNode();
    void broadcastNewFile(const ResourceInfo& resource);
    void broadcastRevokeFile(const ResourceInfo& resource);
    void broadcastFileDeleted(const ResourceInfo& resource);
    void broadcastLogout(const std::vector<ResourceInfo>& resources);

//functions handling broadcasted messages - UDP server
    void handleNewResourceAvailable(char *message);
    void handleOwnerRevokedResource(char *message);
    void handleNodeDeletedResource(char *message);
    void handleNewNodeInNetwork(char *message);
    void handleStateOfNode(char *message);
    void handleNodeLeftNetwork(char *message);


};


#endif //TIN_TORRENTLIKEP2P_TORRENTCLIENT_H
