#ifndef TIN_TORRENTLIKEP2P_TORRENTCLIENT_H
#define TIN_TORRENTLIKEP2P_TORRENTCLIENT_H

#include <vector>
#include <mutex>
#include <map>
#include "ResourceInfo.h"
#include "PeerInfo.h"

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
