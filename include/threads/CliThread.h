#ifndef TIN_TORRENTLIKEP2P_CLITHREAD_H
#define TIN_TORRENTLIKEP2P_CLITHREAD_H

#include <thread>
#include <set>
#include "../structs/Message.h"
#include "../structs/SharedStructs.h"
#include "../structs/SynchronizedFile.h"
#include "TcpThread.h"
#include "UdpThread.h"

class CliThread{

public:

    CliThread(SharedStructs& structs,
              std::unique_ptr<TcpThread>& tcpThread,
              std::unique_ptr<UdpThread>& udpThread):
    sharedStructs(structs), tcpObj(tcpThread), udpObj(udpThread){
        keepGoing = true;
    }

	static void start(CliThread* cliObj){
    	cliObj->runCliThread();
    }

    void runCliThread();
    void terminate();

	void setBarrier(pthread_barrier_t *ptr);

private:
    SharedStructs& sharedStructs;
    std::unique_ptr<TcpThread>& tcpObj;
    std::unique_ptr<UdpThread>& udpObj;
    std::set<int> openSockets;
    std::map<std::string, SynchronizedFile> ongoingDownloadingFiles;
    bool keepGoing;
    pthread_barrier_t* barrier;
    static std::string getResourcePassword();


    void handleClientAddResource(const std::string& , const std::string&);
    void handleClientListResources();
    void handleClientFindResource(const std::string& );
    void handleDownloadResource(const std::string& , const std::string &);
    void handleRevokeResource(const std::string&);


    static std::vector<std::vector<int>> prepareChunkIndices(int peersCount, unsigned int fileSize);
    bool downloadChunksFromPeer(sockaddr_in, const std::vector<int> &, const std::string& , const std::string &);
    void receiveChunks(int sock, int chunksCount, const std::string &filepath);
    void downloadResourceJob(const std::string &resource, const std::string &filepath);
};

#endif //TIN_TORRENTLIKEP2P_CLITHREAD_H
