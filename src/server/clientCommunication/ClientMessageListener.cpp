//
// Created by demongo on 11.05.2021.
//

#include <stdexcept>
#include "../../../include/server/clientCommunication/ClientMessageListener.h"
#include "../../../include/server/workerThreads/downloadWorker/DownloadWorker.h"
#include "../../../include/server/Server.h"

void *ClientMessageListener::run(void *threadInfo) {
	int status;
	void* res;
	thread_info* threadInformation = static_cast<thread_info*>(threadInfo);


	struct thread_info** threads;

	pthread_yield();
	return nullptr;

	// client message handler

	// if client wants download create download worker thread like in main
//	DownloadWorker* downloadWorker = new DownloadWorker();
//	thread_info* downloadWorkerThread = createDownloadWorker(downloadWorker);


	// if client wants to add new local resource
//	FileMetadata* fileMetadata = ((Server*)(threadInformation->parentObj))->addNewLocalResource("resouce name");
//
//	broadcastNewResource(fileMetadata);

	// if client wants network file list
//	sendMessageToClient(((Server*)(threadInformation->parentObj))->getNetworkFiles());

	// if client wants to revoke file
//	((Server*)(threadInformation->parentObj))->revokeFile("file name");
}

thread_info *ClientMessageListener::createDownloadWorker(DownloadWorker *pWorker) {
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
	tinfo->context = pWorker;
//	tinfo->string = workDir;

	status = pthread_create(&tinfo->thread_id, &attr,
							reinterpret_cast<void *(*)(void *)>(&DownloadWorker::start), &tinfo);
	if (status != 0)
		throw std::runtime_error("pthread_create");

	status = pthread_attr_destroy(&attr);
	if (status != 0)
		throw std::runtime_error("pthread_attr_destroy");


	return tinfo;
}
