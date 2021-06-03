#ifndef TIN_TORRENTLIKEP2P_TORRENTCLIENT_H
#define TIN_TORRENTLIKEP2P_TORRENTCLIENT_H

#include "../../include/threads/UdpThread.h"
#include "../../include/threads/TcpThread.h"
#include "../../include/threads/CliThread.h"

class TorrentClient {
public:
    explicit TorrentClient(SharedStructs& structs): tcpObj(structs), udpObj(structs), cliObj(structs, tcpObj, udpObj){ }
    void run();

    void signalHandler();
private:

    CliThread cliObj;
    TcpThread tcpObj;
    UdpThread udpObj;
};


#endif //TIN_TORRENTLIKEP2P_TORRENTCLIENT_H
