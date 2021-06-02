#ifndef TIN_TORRENTLIKEP2P_TORRENTCLIENT_H
#define TIN_TORRENTLIKEP2P_TORRENTCLIENT_H

#include <vector>
#include <mutex>
#include <map>
#include <bits/socket.h>

#include "../structs/Message.h"
#include "../structs/ResourceInfo.h"
#include "../structs/PeerInfo.h"


class TorrentClient {
public:
    TorrentClient(){
        keepGoing = true;
    }

    void run();

    void signalHandler();
private:
    std::map<std::string , ResourceInfo> localResources;
    std::map<std::pair<unsigned long, unsigned short>,std::map<std::string, ResourceInfo> > networkResources;
    std::map<int, struct sockaddr_in> connectedClients;

    std::map<std::string, std::string> filepaths;

    std::mutex localResourcesMutex;
    std::mutex networkResourcesMutex;
    bool keepGoing;
    int udpSocket;
    int tcpSocket;
    struct sockaddr_in broadcastAddress;
    int broadcastSocket;

    const int port = 5555;
    const std::string address = "127.0.0.1";
	const int MAX_FILE_NAME_SIZE = 256;
	void initUdp();
    void initTcp();
    int acceptClient();

    [[noreturn]] void runTcpServerThread();

    [[noreturn]] void runUdpServerThread();
    void runCliThread();

    void handleClientAddResource(const std::string& resourceName, const std::string& resourcePath, const std::string& userPassword);

    void handleClientListResources();

    void handleClientFindResource(const std::string& resourceName);

    void handleDownloadResource(const std::string& resourceName);

    void handleRevokeResource(const std::string& resourceName, const std::string& password);

    void handleExit();

	ClientCommand parseCommand(std::vector<std::string> vecWord, std::string &filepath,
			std::string &resourceName, bool &foundCommand);
	void parseResourceName(std::vector<std::string> vecWord, std::string &resourceName, bool& foundCommand);


//broadcast functions
    void genericBroadcast(UdpMessageCode code, char* payload) const;
    void broadcastNewNode();
    void broadcastNewFile(const ResourceInfo& resource);
    void broadcastRevokeFile(const ResourceInfo& resource);
    void broadcastFileDeleted(const ResourceInfo& resource);
    void broadcastLogout(const std::vector<ResourceInfo>& resources);

//functions handling broadcasted messages - UDP server
    void handleNewResourceAvailable(char *message, sockaddr_in sockaddr);
    void handleOwnerRevokedResource(char *message, sockaddr_in sockaddr);
    void handleNodeDeletedResource(char *message, sockaddr_in sockaddr);

    void handleStateOfNode(char *message, sockaddr_in sockaddr);
    void handleNodeLeftNetwork(char *message, sockaddr_in sockaddr);



    void receive(int socket, bool tcp);


    void handleTcpMessage(char *header, char *payload, int socket);

    void handleUdpMessage(char *header, char *payload, sockaddr_in sockaddr);

    void handleNewNodeInNetwork(char *message, sockaddr_in sockaddr);

    void sendMyState(sockaddr_in in);

    void listResourcesJob();

    void downloadResourceJob(const std::string &resource);

    void findResourceJob(const std::string &resource);

    static std::pair<unsigned long, unsigned short> convertAddress(sockaddr_in address){
        return std::make_pair(address.sin_addr.s_addr, address.sin_port);
    }

    void demandChunkJob(char *payload, int sockaddr);

    void stateBeforeFileTransferJob(char *payload, int sockaddr);

    void chunkTransferJob(char *payload, int sockaddr);

    void errorAfterSyncJob(char *payload, int sockaddr);

    void errorWhileReceivingJob(char *payload, int sockaddr);

    void errorWhileSendingJob(char *payload, int sockaddr);


    void sendChunks(const DemandChunkMessage &message, int socket);

    void sendSync(int socket);

    void sendHeader(int socket, TcpMessageCode code);

    void receiveSync(int socket);

    void clearPeerInfo(int socket);
};

void errno_abort(const std::string& header){
    perror(header.c_str());
    exit(EXIT_FAILURE);
}

#endif //TIN_TORRENTLIKEP2P_TORRENTCLIENT_H
