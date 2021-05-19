//
// Created by demongo on 11.05.2021.
//

#include <cstdlib>
#include <stdexcept>
#include "../../include/server/Server.h"


void* Server::run(void *threadInfo) {

	int status;
	void* res;

	initializeLocalResources(static_cast<thread_info*>(threadInfo)->string);

	broadcastAddNode();

	struct thread_info** threads;
	// listening to other nodes response
	NodeMessageListener* nodeMessageListener = new NodeMessageListener();
	thread_info* nodeMessageListenerThread = createNodeMessageListener(nodeMessageListener);
    // listening to client messages
    ClientMessageListener* clientMessageListener = new ClientMessageListener();
    thread_info* clientMessageListenerThread = createClientMessageListener(clientMessageListener);

	// maybe not needed, just create worker thread when client message receved
	// on the other hand may be useful for download
	//struct thread_info* SupervisorWorker = createSupervisorWorker();

	// for directory check and local resources integrity
	IntegrityWorker* integrityWorker = new IntegrityWorker();
	thread_info* integrityWorkerThread = createIntegrityWorker(integrityWorker);


	// can be done in loop with threads array
	status = pthread_join(nodeMessageListenerThread->thread_id, &res);
	if(status != 0){
		throw std::runtime_error("pthread_join");
	}
	free(nodeMessageListenerThread);

	status = pthread_join(clientMessageListenerThread->thread_id, &res);
	if(status != 0){
		throw std::runtime_error("pthread_join");
	}
	free(clientMessageListenerThread);

	status = pthread_join(integrityWorkerThread->thread_id, &res);
	if(status != 0){
		throw std::runtime_error("pthread_join");
	}
	free(integrityWorkerThread);

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

thread_info* Server::createNodeMessageListener(NodeMessageListener* nodeMessageListener) {
	pthread_attr_t attr;
	int status;

	status = pthread_attr_init(&attr);
	if(status != 0){
		throw std::runtime_error("pthread_attr_init!");
	}

	thread_info *tinfo = static_cast<thread_info *>(calloc(1, sizeof(*tinfo)));
	if (tinfo == NULL)
		throw std::runtime_error("calloc");

	tinfo->thread_num = 3;
	tinfo->context = nodeMessageListener;
	tinfo->parentObj = this;
//	tinfo->string = workDir;

	status = pthread_create(&tinfo->thread_id, &attr,
							reinterpret_cast<void *(*)(void *)>(&NodeMessageListener::start), &tinfo);
	if (status != 0)
		throw std::runtime_error("pthread_create");

	status = pthread_attr_destroy(&attr);
	if (status != 0)
		throw std::runtime_error("pthread_attr_destroy");


	return tinfo;
}

thread_info *Server::createClientMessageListener(ClientMessageListener* clientMessageListener) {
	pthread_attr_t attr;
	int status;

	status = pthread_attr_init(&attr);
	if(status != 0){
		throw std::runtime_error("pthread_attr_init!");
	}

	thread_info *tinfo = static_cast<thread_info *>(calloc(1, sizeof(*tinfo)));
	if (tinfo == NULL)
		throw std::runtime_error("calloc");

	tinfo->thread_num =4;
	tinfo->context = clientMessageListener;
	tinfo->parentObj = this;
//	tinfo->string = workDir;

	status = pthread_create(&tinfo->thread_id, &attr,
							reinterpret_cast<void *(*)(void *)>(&ClientMessageListener::start), &tinfo);
	if (status != 0)
		throw std::runtime_error("pthread_create");

	status = pthread_attr_destroy(&attr);
	if (status != 0)
		throw std::runtime_error("pthread_attr_destroy");


	return tinfo;
}

thread_info *Server::createIntegrityWorker(IntegrityWorker* integrityWorker) {
	pthread_attr_t attr;
	int status;

	status = pthread_attr_init(&attr);
	if(status != 0){
		throw std::runtime_error("pthread_attr_init!");
	}

	thread_info *tinfo = static_cast<thread_info *>(calloc(1, sizeof(*tinfo)));
	if (tinfo == NULL)
		throw std::runtime_error("calloc");

	tinfo->thread_num = 5;
	tinfo->context = integrityWorker;
	tinfo->parentObj = this;
//	tinfo->string = workDir;

	status = pthread_create(&tinfo->thread_id, &attr,
							reinterpret_cast<void *(*)(void *)>(&IntegrityWorker::start), &tinfo);
	if (status != 0)
		throw std::runtime_error("pthread_create");

	status = pthread_attr_destroy(&attr);
	if (status != 0)
		throw std::runtime_error("pthread_attr_destroy");


	return tinfo;
}

FileMetadata* Server::addNewLocalResource(std::string name) {
	return nullptr;
}


