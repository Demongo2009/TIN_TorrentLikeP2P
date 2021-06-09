#ifndef TIN_TORRENTLIKEP2P_TORRENTCLIENT_H
#define TIN_TORRENTLIKEP2P_TORRENTCLIENT_H

#include <memory>
#include "../../include/threads/UdpThread.h"
#include "../../include/threads/TcpThread.h"
#include "../../include/threads/CliThread.h"

class TorrentClient {
public:
    explicit TorrentClient(SharedStructs& structs){
    	tcpObj = std::make_unique<TcpThread>(TcpThread(structs));
    	udpObj = std::make_unique<UdpThread>(UdpThread(structs));
    	cliObj = std::make_unique<CliThread>(CliThread(structs, tcpObj, udpObj));
    }
    void run();

    void signalHandler();
private:

    std::unique_ptr<CliThread> cliObj;
    std::unique_ptr<TcpThread> tcpObj;
    std::unique_ptr<UdpThread> udpObj;
};


#endif //TIN_TORRENTLIKEP2P_TORRENTCLIENT_H
