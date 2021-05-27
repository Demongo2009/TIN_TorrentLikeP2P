//
// Created by konrad on 5/27/21.
//

#ifndef TIN_TORRENTLIKEP2P_TORRENTCLIENT_H
#define TIN_TORRENTLIKEP2P_TORRENTCLIENT_H


#include <iostream>

class TorrentClient {
private:
    // TODO:
    //  struktury, które będą współdzielone pomiędzy wątkami głównymi
    //  mutexy ktore synchronizuja dostep do tych sturktur


    void init();
    void* runServerThread();
    /**
     * server specific thread/functions:
     * -send chunk
     * -revoke file - just set is_revoked bit to true -
     * -synchronize with other node - check if you and the other has the same information about THE resource(ONLY) that one want to download
     *
     */

    void* runCliThread();
    /**
     * cli specific threads/functions:
     * -print local files(not shared yet)
     * -print local files(shared)
     * -print available to download files(all - already downloaded)
     * -revoke file(check hash and then broadcast)
     * -download file
     * */
public:
    void run();
};


#endif //TIN_TORRENTLIKEP2P_TORRENTCLIENT_H
