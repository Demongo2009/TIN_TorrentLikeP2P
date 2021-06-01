#ifndef TIN_TORRENTLIKEP2P_RESOURCEINFO_H
#define TIN_TORRENTLIKEP2P_RESOURCEINFO_H

#include <string>

struct ResourceInfo {
    std::string resourceName;
    unsigned int sizeInBytes;
    std::size_t revokeHash;
    bool isRevoked;
};


#endif //TIN_TORRENTLIKEP2P_RESOURCEINFO_H
