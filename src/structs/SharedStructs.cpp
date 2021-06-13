#include <arpa/inet.h>
#include "../../include/structs/SharedStructs.h"
#include "../../include/utils/utils.h"

void SharedStructs::addNetworkResource(sockaddr_in sockaddr, const ResourceInfo& resource) {
    networkResourcesMutex.lock();
    networkResources[convertAddressLong(sockaddr)][resource.resourceName] = resource;
    networkResourcesMutex.unlock();
}

void SharedStructs::deleteNetworkResource(const std::string& resourceName) {
    networkResourcesMutex.lock();
    for(auto& resources: networkResources){
        auto it = resources.second.find(resourceName);
        if( it != resources.second.end()){
            resources.second.erase(resourceName);
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

std::size_t SharedStructs::getRevokeHash(const std::string &resourceName) {
    localResourcesMutex.lock();
    std::size_t revokeHash = localResources.at(resourceName).revokeHash;
    localResourcesMutex.unlock();
    return revokeHash;
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

void SharedStructs::printLocalResources() {
    localResourcesMutex.lock();
    for(const auto& it : localResources){
        std::cout<< "NAME: "<<it.first<< " SIZE: "<<it.second.sizeInBytes<<std::endl;
    }
    localResourcesMutex.unlock();
}

void SharedStructs::printNetworkResources() {
    networkResourcesMutex.lock();
    struct in_addr addr{};
    for(const auto& [peerAddress, resources] : networkResources){
        addr.s_addr = peerAddress;
        std::cout<< "RESOURCES OF PEER: "<<inet_ntoa(addr) <<std::endl;
        for(const auto& it: resources){
            std::cout<< "NAME: "<<it.first<< " SIZE: "<<it.second.sizeInBytes<<std::endl;
        }
    }
    networkResourcesMutex.unlock();
}

void SharedStructs::findLocalResource(const std::string& resourceName) {
    localResourcesMutex.lock();
    auto it = localResources.find(resourceName);
    if( it != localResources.end()){
        std::cout<<"LOCAL RESOURCE: "<< resourceName<< " PATH: " << filepaths.at(resourceName) << std::endl;
    }
    localResourcesMutex.unlock();
}

void SharedStructs::findNetworkResource(const std::string &resourceName) {
    struct in_addr addr{};
    for(auto& [peerAddress, resources]: networkResources){
        auto it = resources.find(resourceName);
        if( it != resources.end()){
            addr.s_addr = peerAddress;
            std::cout<< "NETWORK RESOURCE OF PEER: "<<inet_ntoa(addr) <<std::endl;
            std::cout<<"NAME: "<< resourceName<< " SIZE: " << it->second.sizeInBytes << std::endl;
        }
    }
    networkResourcesMutex.unlock();
}

bool SharedStructs::checkIfFileIsInPossession(const std::string& resourceName) {
    localResourcesMutex.lock();
    auto it = localResources.find(resourceName);
    if( it != localResources.end()){
        std::cout<<"YOU ARE ALREADY IN POSSESSION OF THE RESOURCE "<< resourceName
                 << " PATH: " << filepaths.at(resourceName) << std::endl;
        localResourcesMutex.unlock();
        return false;
    }
    localResourcesMutex.unlock();
    return true;
}

std::vector<struct sockaddr_in> SharedStructs::getNodesContainingResource(const std::string &resourceName) {
    std::vector<struct sockaddr_in> peers;
    struct sockaddr_in addr{};
    for(auto& [peerAddress, resources] : networkResources){
        auto it = resources.find(resourceName);
        if( it != resources.end()){
            addr.sin_addr.s_addr = peerAddress;
            addr.sin_port = ntohs(5555);
            addr.sin_family = AF_INET;
            peers.emplace_back(addr);
        }
    }
    networkResourcesMutex.unlock();
    return peers;
}

bool SharedStructs::deleteLocalResource(const std::string &resourceName, std::size_t hash) {
    localResourcesMutex.lock();
    if(localResources.find(resourceName) == localResources.end()){
        localResourcesMutex.unlock();
        std::cout<<"No such resource"<<std::endl;
        return false;
    }
    if(localResources.at(resourceName).revokeHash != hash ){
        localResourcesMutex.unlock();
        std::cout<<"Invalid revoke password!"<<std::endl;
        return false;
    }
    localResources.erase(resourceName);
    localResourcesMutex.unlock();
    return true;
}

bool SharedStructs::validateChunks(const std::string &resourceName, const std::vector<unsigned long> &chunkIndices) {
    localResourcesMutex.lock();
    if(localResources.find(resourceName) == localResources.end() ) {
        localResourcesMutex.unlock();
        return false;
    }
    unsigned long fileSize = localResources.at(resourceName).sizeInBytes;
    int c = CHUNK_SIZE;
    for(const auto & index : chunkIndices) {
        unsigned long offset = index * c;
        if (offset > fileSize){
            localResourcesMutex.unlock();
            return false;
        }
    }
    localResourcesMutex.unlock();
    return true;
}

bool SharedStructs::deleteLocalResource(const std::string &resourceName) {
	localResourcesMutex.lock();
	if(localResources.find(resourceName) == localResources.end()){
		localResourcesMutex.unlock();
		return false;
	}
	localResources.erase(resourceName);
	localResourcesMutex.unlock();
	return true;
}



