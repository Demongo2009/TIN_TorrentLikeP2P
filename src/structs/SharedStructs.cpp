#include "../../include/structs/SharedStructs.h"
#include "../../include/utils.h"

void SharedStructs::addNetworkResource(sockaddr_in sockaddr, const ResourceInfo& resource) {
    networkResourcesMutex.lock();
    networkResources[convertAddressLong(sockaddr)][resource.resourceName] = resource;
    networkResourcesMutex.unlock();
}

void SharedStructs::revokeResource(const ResourceInfo& resource) {
    networkResourcesMutex.lock();
    for(auto& resources: networkResources){
        auto it = resources.second.find(resource.resourceName);
        if( it != resources.second.end()){
            resources.second.erase(resource.resourceName);
        }
    }
    networkResourcesMutex.unlock();
}

void SharedStructs::deleteResourceFromNode(sockaddr_in sockaddr, const ResourceInfo &resource) {
    networkResourcesMutex.lock();
    networkResources[convertAddressLong(sockaddr)].erase(resource.resourceName);
    networkResourcesMutex.unlock();
}
