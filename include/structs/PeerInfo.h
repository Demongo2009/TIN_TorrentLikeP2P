#ifndef TIN_TORRENTLIKEP2P_PEERINFO_H
#define TIN_TORRENTLIKEP2P_PEERINFO_H

#include <vector>
#include <string>

class PeerInfo {
    int socketId;//int wystarczy czy cos innego?
    std::vector<std::string> resources;
};

#endif //TIN_TORRENTLIKEP2P_PEERINFO_H
