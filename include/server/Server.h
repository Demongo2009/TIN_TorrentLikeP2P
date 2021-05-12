//
// Created by demongo on 11.05.2021.
//

#ifndef TIN_TORRENTLIKEP2P_SERVER_H
#define TIN_TORRENTLIKEP2P_SERVER_H


#include <unordered_map>
#include <vector>
#include <openssl/md5.h>
#include <cstring>
#include <netinet/in.h>
#include "nodesCommunication/NodeMessageListener.h"
#include "clientCommunication/ClientMessageListener.h"
#include "workerThreads/integrityWorker/IntegrityWorker.h"
#include "../Thread.h"

class FileMetadata{
	std::string name;
	long int chunkSize;

	// ive chosen hash md5 but can be some other
	unsigned char revokeHash[MD5_DIGEST_LENGTH];
	bool isRevoked;
	bool isAvailableLocally;

public:
	FileMetadata(const std::string &name, long chunkSize, const unsigned char * revokeHash, bool isRevoked,
				 bool isAvailableLocally) : name(name), chunkSize(chunkSize),
											isRevoked(isRevoked), isAvailableLocally(isAvailableLocally) {
		strcpy(reinterpret_cast<char *>(this->revokeHash), reinterpret_cast<const char *>(revokeHash));
	}

	const std::string &getName() const {
		return name;
	}

	void setName(const std::string &name) {
		FileMetadata::name = name;
	}

	long getChunkSize() const {
		return chunkSize;
	}

	void setChunkSize(long chunkSize) {
		FileMetadata::chunkSize = chunkSize;
	}

	const unsigned char *getRevokeHash() const {
		return revokeHash;
	}

	bool isRevoked1() const {
		return isRevoked;
	}

	void setIsRevoked(bool isRevoked) {
		FileMetadata::isRevoked = isRevoked;
	}

	bool isAvailableLocally1() const {
		return isAvailableLocally;
	}

	void setIsAvailableLocally(bool isAvailableLocally) {
		FileMetadata::isAvailableLocally = isAvailableLocally;
	}
};

class NodeMetadata{
	std::string name;
	int sockFD;
	struct in_addr nodeAddr;
	bool isOnline;

public:
	NodeMetadata(const std::string &name, int sockFd, const in_addr &nodeAddr, bool isOnline) :
			name(name),
			sockFD(sockFd),
			nodeAddr(
					nodeAddr),
			isOnline(
					isOnline) {}
};

class Server {

	class LocalResources{
		std::string currentWorkingDir;
		std::unordered_map<std::string, FileMetadata> filesData;
		std::vector<std::string> localFiles;
		std::vector<std::string> networkFiles;
		std::unordered_map<int, NodeMetadata> nodesData;

	public:

		LocalResources(){}

		LocalResources(const std::string &currentWorkingDir,
					   const std::unordered_map<std::string, FileMetadata> &filesData,
					   const std::vector<std::string> &localFiles, const std::vector<std::string> &networkFiles,
					   const std::unordered_map<int, NodeMetadata> &nodesData) : currentWorkingDir(currentWorkingDir),
																				 filesData(filesData),
																				 localFiles(localFiles),
																				 networkFiles(networkFiles),
																				 nodesData(nodesData) {}

		const std::string &getCurrentWorkingDir() const {
			return currentWorkingDir;
		}

		void setCurrentWorkingDir(const std::string &currentWorkingDir) {
			LocalResources::currentWorkingDir = currentWorkingDir;
		}

		const std::unordered_map<std::string, FileMetadata> &getFilesData() const {
			return filesData;
		}

		void setFilesData(const std::unordered_map<std::string, FileMetadata> &filesData) {
			LocalResources::filesData = filesData;
		}

		const std::vector<std::string> &getLocalFiles() const {
			return localFiles;
		}

		void setLocalFiles(const std::vector<std::string> &localFiles) {
			LocalResources::localFiles = localFiles;
		}

		const std::vector<std::string> &getNetworkFiles() const {
			return networkFiles;
		}

		void setNetworkFiles(const std::vector<std::string> &networkFiles) {
			LocalResources::networkFiles = networkFiles;
		}

		const std::unordered_map<int, NodeMetadata> &getNodesData() const {
			return nodesData;
		}

		void setNodesData(const std::unordered_map<int, NodeMetadata> &nodesData) {
			LocalResources::nodesData = nodesData;
		}
	};


	LocalResources localResources;
	struct in_addr  serverAddress;
	int serverPort;

	void initializeLocalResources(const std::string& workDir);

	void broadcastAddNode();

	thread_info* createNodeMessageListener(NodeMessageListener* nodeMessageListener);

	thread_info *createClientMessageListener(ClientMessageListener* clientMessageListener);

	thread_info *createIntegrityWorker(IntegrityWorker* integrityWorker);


public:
	Server(){}

	static void *start(void *threadInfo){
		return ((Server*)(static_cast<thread_info*>(threadInfo)->context))->run(threadInfo);
	}

	void *run(void *threadInfo);

	FileMetadata* addNewLocalResource(std::string name);

	std::vector<std::string> getNetworkFiles(){
		return localResources.getNetworkFiles();
	}

	bool revokeFile(std::string name);
};


#endif //TIN_TORRENTLIKEP2P_SERVER_H
