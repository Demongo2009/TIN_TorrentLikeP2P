#ifndef TIN_TORRENTLIKEP2P_RESOURCEINFO_H
#define TIN_TORRENTLIKEP2P_RESOURCEINFO_H

#include <string>

class ResourceInfo {
    std::string resourceName;
    unsigned int sizeInBytes;
    std::string revokeHash;
    bool isRevoked; //nie wiem czy to potrzebne, bo jak bedzie revoke to rownie dobrze mozemy usunac tą strukturę
};


#endif //TIN_TORRENTLIKEP2P_RESOURCEINFO_H
