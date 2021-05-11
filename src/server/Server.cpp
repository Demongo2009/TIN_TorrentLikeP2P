//
// Created by demongo on 11.05.2021.
//

#include <cstdlib>
#include <stdexcept>
#include "../../include/server/Server.h"
#include "../../include/Thread.h"

void Server::runServer(void *threadInfo) {

	int status;
	void* res;

	initializeLocalResources(static_cast<thread_info*>(threadInfo)->string);

	broadcastAddNode();

	struct thread_info** threads;
	// listening to other nodes response
	struct thread_info* nodeMessageListener = createNodeMessageListener();
    // listening to client messages
	struct thread_info* clientMessageListener = createClientMessageListener();

	// maybe not needed, just create worker thread when client message receved
	// on the other hand may be useful for download
	//struct thread_info* SupervisorWorker = createSupervisorWorker();

	// for directory check and local resources integrity
	struct thread_info* integrityWorker = createIntegrityWorker();


	// can be done in loop with threads array
	status = pthread_join(nodeMessageListener->thread_id, &res);
	if(status != 0){
		throw std::runtime_error("pthread_join");
	}
	free(nodeMessageListener);

	status = pthread_join(clientMessageListener->thread_id, &res);
	if(status != 0){
		throw std::runtime_error("pthread_join");
	}
	free(clientMessageListener);

	status = pthread_join(integrityWorker->thread_id, &res);
	if(status != 0){
		throw std::runtime_error("pthread_join");
	}
	free(integrityWorker);

	free(res);
}

void Server::initializeLocalResources(const std::string& workDir) {
	// here we can read metadata from local file?
	const std::unordered_map<std::string, FileMetadata> filesData;
	const std::vector<std::string> localFiles;
	const std::vector<std::string> networkFiles;
	const std::unordered_map<int, NodeMetadata> nodesData;
	localResources = LocalResources(workDir,filesData,localFiles,networkFiles,nodesData);
}

void Server::broadcastAddNode() {
	//send serverAddr and port
}

struct thread_info* Server::createNodeMessageListener() {
	return nullptr;
}

thread_info *Server::createClientMessageListener() {
	return nullptr;
}

thread_info *Server::createIntegrityWorker() {
	return nullptr;
}


