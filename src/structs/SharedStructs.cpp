#include "../../include/structs/SharedStructs.h"
#include "../../include/utils/utils.h"

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

unsigned long SharedStructs::getFileSize(const std::string& resourceName) {
    localResourcesMutex.lock();
    unsigned long fileSize = localResources.at(resourceName).sizeInBytes;
    localResourcesMutex.unlock();
    return fileSize;
}

bool SharedStructs::addLocalResource(const ResourceInfo &resource,const std::string& filepath) {
	std::string resourceName = resource.resourceName;
	localResourcesMutex.lock();
	if(localResources.find(resourceName) != localResources.end()){
		std::cout << "File of this name already exists!\n";
		localResourcesMutex.unlock();
		return false;
	}
	for(auto& resources: networkResources){
		auto it = resources.second.find(resourceName);
		if( it != resources.second.end()){
			std::cout << "File of this name already exists!\n";
			localResourcesMutex.unlock();
			return false;
		}
	}
	localResources.emplace(resourceName, resource);
	localResourcesMutex.unlock();
	filepaths.insert(std::make_pair(resourceName, filepath));
	return true;
}

std::vector<std::string> SharedStructs::getLocalStateString() {
    std::stringstream ss;
    std::vector<std::string> payloads;
    localResourcesMutex.lock();

    for(const auto& [resourceName, resource] : localResources){
        if(ss.str().size() + resourceName.size() + sizeof(long) + sizeof(int) > MAX_SIZE_OF_PAYLOAD){
            payloads.emplace_back(ss.str());
            ss.clear();
        }
        ss << resource.resourceName  << SEPARATOR
           << resource.revokeHash  << SEPARATOR
           << resource.sizeInBytes  << SEPARATOR;
    }

    localResourcesMutex.unlock();
    payloads.emplace_back(ss.str());
    return std::move(payloads);
}

