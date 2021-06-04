#ifndef TIN_TORRENTLIKEP2P_TORRENTCLIENT_H
#define TIN_TORRENTLIKEP2P_TORRENTCLIENT_H

#include "../../include/threads/UdpThread.h"
#include "../../include/threads/TcpThread.h"
#include "../../include/threads/CliThread.h"

class TorrentClient {
public:
    explicit TorrentClient(SharedStructs& structs){
    	tcpObj = new TcpThread(structs);
    	udpObj = new UdpThread(structs),
    	cliObj = new CliThread(structs, tcpObj, udpObj);
    }
    void run();

    void signalHandler();
private:

    CliThread* cliObj;
    TcpThread* tcpObj;
    UdpThread* udpObj;
};


#endif //TIN_TORRENTLIKEP2P_TORRENTCLIENT_H
