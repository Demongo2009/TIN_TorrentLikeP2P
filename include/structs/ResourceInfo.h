#ifndef TIN_TORRENTLIKEP2P_RESOURCEINFO_H
#define TIN_TORRENTLIKEP2P_RESOURCEINFO_H

#include <string>

struct ResourceInfo {
    std::string resourceName;
    unsigned int sizeInBytes;
    std::size_t revokeHash;
    bool isRevoked;
    ResourceInfo(std::string resourceName="",
                 unsigned int sizeInBytes=0,
                 std::size_t revokeHash=0,
                 bool isRevoked = false):
                    resourceName(std::move(resourceName)),
                    sizeInBytes(sizeInBytes),
                    revokeHash(revokeHash),
                    isRevoked(isRevoked) {}

};


#endif //TIN_TORRENTLIKEP2P_RESOURCEINFO_H
