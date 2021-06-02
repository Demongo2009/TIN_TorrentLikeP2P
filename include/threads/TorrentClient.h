#ifndef TIN_TORRENTLIKEP2P_TORRENTCLIENT_H
#define TIN_TORRENTLIKEP2P_TORRENTCLIENT_H

#include <vector>
#include <mutex>
#include <map>
#include <sys/socket.h>

#include "../structs/Message.h"
#include "../structs/ResourceInfo.h"
#include "../structs/ConnectedPeerInfo.h"


class TorrentClient {
public:
    TorrentClient(){
        keepGoing = true;
    }

    void run();
    void debug(ResourceInfo&);

    void signalHandler();
private:
    std::map<std::string , ResourceInfo> localResources;
    std::map<std::pair<unsigned long, unsigned short>,std::map<std::string, ResourceInfo> > networkResources;
    std::map<int, ConnectedPeerInfo> connectedClients;

    std::map<std::string, std::string> filepaths;

    std::mutex localResourcesMutex;
    std::mutex networkResourcesMutex;
    std::mutex syncMutex;
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

    void handleDownloadResource(const std::string& resourceName, const std::string &filepath);

    void handleRevokeResource(const std::string& resourceName, const std::string& password);

    void handleExit() const;

	ClientCommand parseCommand(std::vector<std::string> vecWord, std::string &filepath,
			std::string &resourceName, bool &foundCommand);
	void parseResourceName(std::vector<std::string> vecWord, std::string &resourceName, bool& foundCommand) const;


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
    void handleNodeLeftNetwork(sockaddr_in sockaddr);



    void receive(int socket, bool tcp);

    void handleTcpMessage(char *header, char *payload, int socket);

    void handleUdpMessage(char *header, char *payload, sockaddr_in sockaddr);

    void handleNewNodeInNetwork(sockaddr_in sockaddr);

    void sendMyState(sockaddr_in in);

    void downloadResourceJob(const std::string &resource, const std::string &filepath);


    static std::pair<unsigned long, unsigned short> convertAddress(sockaddr_in address){
        return std::make_pair(address.sin_addr.s_addr, address.sin_port);
    }


    void demandChunkJob(char *payload, int sockaddr);

    void sendChunks(const DemandChunkMessage &message, int socket);

    void sendSync(int socket);

    static void sendHeader(int socket, TcpMessageCode code);

    void receiveSync(int socket);

    void clearPeerInfo(int socket);

    //PUBLIC TYLKO NA POTRZEBY DEBUGOWANIA, PO TESTACH MOZNA WYWALIC
public:
    //deserialization
    static ResourceInfo deserializeResource(const char *message,
                                            bool toVector=false,
                                            int *pointer = nullptr);
    static std::vector<ResourceInfo> deserializeVectorOfResources(char *message);

    static DemandChunkMessage deserializeChunkMessage(const char *message);

    static std::vector<std::vector<int>> prepareChunkIndices(int peersCount, unsigned int fileSize);

    void downloadChunksFromPeer(sockaddr_in, const std::vector<int> &chunksIndices, const std::string &filepath);

    void receiveChunks(int socket, int chunksCount, const std::string &filepath);


    static void writeFile(const char *payload, unsigned int size, const std::string &filepath);
};
void errno_abort(const std::string& header);

#endif //TIN_TORRENTLIKEP2P_TORRENTCLIENT_H
