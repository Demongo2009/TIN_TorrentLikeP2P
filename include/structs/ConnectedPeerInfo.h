#ifndef TIN_TORRENTLIKEP2P_CONNECTEDPEERINFO_H
#define TIN_TORRENTLIKEP2P_CONNECTEDPEERINFO_H

#include <vector>
#include <string>
#include <netinet/in.h>

struct ConnectedPeerInfo {
    ConnectedPeerInfo() {

    }

    ConnectedPeerInfo(const ConnectedPeerInfo& other){
        address = other.address;
        isSync = other.isSync;
        threadsCount = other.threadsCount;
    }

    struct sockaddr_in address;
    bool isSync;
    bool isFinished;
    unsigned char threadsCount;
    ConnectedPeerInfo(struct sockaddr_in address) : address(address) {
        isSync = false;
        threadsCount = 0;
    }
};

#endif //TIN_TORRENTLIKEP2P_CONNECTEDPEERINFO_H
