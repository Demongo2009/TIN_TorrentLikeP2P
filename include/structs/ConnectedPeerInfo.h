#ifndef TIN_TORRENTLIKEP2P_CONNECTEDPEERINFO_H
#define TIN_TORRENTLIKEP2P_CONNECTEDPEERINFO_H

#include <vector>
#include <string>
#include <netinet/in.h>

struct ConnectedPeerInfo {

    ConnectedPeerInfo(const ConnectedPeerInfo& other){
        address = other.address;
        isSync = other.isSync;
    }

    struct sockaddr_in address;
    bool isSync;

    explicit ConnectedPeerInfo(struct sockaddr_in address) : address(address) {
        isSync = false;
    }
};

#endif //TIN_TORRENTLIKEP2P_CONNECTEDPEERINFO_H
