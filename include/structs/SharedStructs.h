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


    void addNetworkResource(sockaddr_in, const ResourceInfo&);
    void deleteNetworkResource(const std::string&);

    void addNetworkNode(sockaddr_in);
    void deleteNetworkNode(sockaddr_in);

    void deleteResourceFromNode(sockaddr_in, const ResourceInfo&);
    void registerNewNodeWithItsResources(sockaddr_in, const std::vector<ResourceInfo>& );

    bool addLocalResource(const ResourceInfo &resource, const std::string &filepath, bool);
    bool deleteLocalResourceAfterRevoke(const std::string&, std::size_t);
    bool deleteLocalResource(const std::string&);
    std::vector<std::string> getLocalStateString();
    unsigned long getFileSize(const std::string&);
    std::size_t getRevokeHash(const std::string&);
    void printLocalResources();
    void printNetworkResources();
    void findLocalResource(const std::string &);
    void findNetworkResource(const std::string &);
    bool checkIfFileIsInPossession(const std::string& );
    std::vector<struct sockaddr_in> getNodesContainingResource(const std::string& );
    bool validateChunks(const std::string&, const std::vector<unsigned long>&);



};

#endif //TIN_TORRENTLIKEP2P_SHAREDSTRUCTS_H
