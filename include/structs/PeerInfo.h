#ifndef TIN_TORRENTLIKEP2P_PEERINFO_H
#define TIN_TORRENTLIKEP2P_PEERINFO_H

#include <vector>
#include <string>
#include <netinet/in.h>

class PeerInfo {
    int socketId;//int wystarczy czy cos innego?
    struct sockaddr_in address;
    std::vector<std::string> resources;

public:
    PeerInfo(struct sockaddr_in address) : address(address) {

    }
};

#endif //TIN_TORRENTLIKEP2P_PEERINFO_H
