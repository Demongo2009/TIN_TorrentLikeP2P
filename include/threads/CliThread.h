//
// Created by bartlomiej on 03.06.2021.
//

#ifndef TIN_TORRENTLIKEP2P_CLITHREAD_H
#define TIN_TORRENTLIKEP2P_CLITHREAD_H

#include <thread>
#include <set>
#include "../structs/Message.h"
#include "../structs/SharedStructs.h"
#include "TcpThread.h"
#include "UdpThread.h"


class CliThread{

public:

    CliThread(SharedStructs& structs, TcpThread* tcpThread, UdpThread* udpThread) : sharedStructs(structs), tcpObj(tcpThread),
                                                                                    udpObj(udpThread){
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
    TcpThread* tcpObj;
    UdpThread* udpObj;
    std::set<int> openSockets;
    std::set<std::string> ongoingDowloadingFilepaths;
    bool keepGoing;
    const int MAX_FILE_NAME_SIZE = 256;
    pthread_barrier_t* barrier;



    void handleClientAddResource(const std::string& resourceName, const std::string& resourcePath, const std::string& userPassword);

    void handleClientListResources();

    void handleClientFindResource(const std::string& resourceName);

    void handleDownloadResource(const std::string& resourceName, const std::string &filepath);

    void handleRevokeResource(const std::string& resourceName, const std::string& password);



    ClientCommand parseCommand(std::vector<std::string> vecWord, std::string &filepath,
                               std::string &resourceName, bool &foundCommand);
    void parseResourceName(std::vector<std::string> vecWord, std::string &resourceName, bool& foundCommand) const;

    static std::vector<std::vector<int>> prepareChunkIndices(int peersCount, unsigned int fileSize);

    void downloadChunksFromPeer(sockaddr_in, const std::vector<int> &chunksIndices, const std::string &filepath);

    void receiveChunks(int sock, int chunksCount, const std::string &filepath);


    static void writeFile(const char *payload, unsigned int size, const std::string &filepath);

    void downloadResourceJob(const std::string &resource, const std::string &filepath);
};

#endif //TIN_TORRENTLIKEP2P_CLITHREAD_H
