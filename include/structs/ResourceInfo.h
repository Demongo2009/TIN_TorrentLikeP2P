#ifndef TIN_TORRENTLIKEP2P_RESOURCEINFO_H
#define TIN_TORRENTLIKEP2P_RESOURCEINFO_H

#include <string>
#include <stdexcept>
#include <sstream>
#include "Message.h"

struct ResourceInfo {
    std::string resourceName;
    unsigned long long sizeInBytes;
    std::size_t revokeHash;
    bool isRevoked;
    explicit ResourceInfo(std::string resourceName="",
                 unsigned long long sizeInBytes=0,
                 std::size_t revokeHash=0,
                 bool isRevoked = false):
                    resourceName(std::move(resourceName)),
                    sizeInBytes(sizeInBytes),
                    revokeHash(revokeHash),
                    isRevoked(isRevoked) {}

    static ResourceInfo deserializeResource(const char *message, bool toVector = false,int *dataPointer = nullptr);
    static std::vector<ResourceInfo> deserializeVectorOfResources(char *message);

};


#endif //TIN_TORRENTLIKEP2P_RESOURCEINFO_H
