#ifndef TIN_TORRENTLIKEP2P_SHAREDSTRUCTS_H
#define TIN_TORRENTLIKEP2P_SHAREDSTRUCTS_H

#include <mutex>
#include <map>
#include <netinet/in.h>
#include "ResourceInfo.h"

struct SharedStructs{
    std::map<std::string , ResourceInfo> localResources;
    std::map<unsigned long,std::map<std::string, ResourceInfo> > networkResources;
    std::mutex localResourcesMutex;
    std::mutex networkResourcesMutex;
    std::map<std::string, std::string> filepaths;

    //UDP
    void addNetworkResource(sockaddr_in, const ResourceInfo&);
    void revokeResource(const ResourceInfo&);
    void deleteResourceFromNode(sockaddr_in, const ResourceInfo&);
    //4. addNewNode
    //5. actualizeNodeState
    //....

    //TCP
    //..

    //CLI
    //...


};

#endif //TIN_TORRENTLIKEP2P_SHAREDSTRUCTS_H
