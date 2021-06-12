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

void SharedStructs::addNetworkNode(sockaddr_in sockaddr) {
    networkResourcesMutex.lock();
    networkResources[convertAddressLong(sockaddr)] = std::map<std::string, ResourceInfo>();
    networkResourcesMutex.unlock();
}

void SharedStructs::registerNewNodeWithItsResources(sockaddr_in sockaddr, const std::vector<ResourceInfo>& resources) {
    networkResourcesMutex.lock();
    if( resources.empty() ){
        networkResources[convertAddressLong(sockaddr)] = std::map<std::string, ResourceInfo>();
    }
    for(const auto & r : resources){
        networkResources[convertAddressLong(sockaddr)][r.resourceName] = r;
    }
    networkResourcesMutex.unlock();
}

void SharedStructs::deleteNetworkNode(sockaddr_in sockaddr) {
    networkResourcesMutex.lock();
    networkResources.erase(convertAddressLong(sockaddr));
    networkResourcesMutex.unlock();
}

std::string SharedStructs::getMyStateString() {
    //TODO nie wiem czy w wysylaniu swojego staniu nie ma buga  anie jestem w stanie tego sprawdzić ;(
    return std::__cxx11::string();
}

unsigned long SharedStructs::getFileSize(const std::string& resourceName) {
    localResourcesMutex.lock();
    unsigned long fileSize = localResources.at(resourceName).sizeInBytes;
    localResourcesMutex.unlock();
    return fileSize;
}

