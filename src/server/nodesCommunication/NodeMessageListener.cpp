//
// Created by demongo on 11.05.2021.
//

#include <stdexcept>
#include "../../../include/server/nodesCommunication/NodeMessageListener.h"
#include "../../../include/server/workerThreads/downloadWorker/DownloadWorker.h"
#include "../../../include/server/workerThreads/uploadWorker/UploadWorker.h"

void *NodeMessageListener::run(void *threadInfo) {

	int status;
	void* res;
	thread_info* threadInformation = static_cast<thread_info*>(threadInfo);

	struct thread_info** threads;


	// node message handler / whole UDP(broadcast) site of protocol handler needed
	// TCP site of protocol handled in supervisor or download/upload workers
	// best to call functions like: (if it works xd)
	// ((Server*)(threadInformation->parentObj))-> <method_name>


	// listen

	// if download create downloadWorker
//	DownloadWorker* downloadWorker = new DownloadWorker();
//	thread_info* downloadWorkerThread = createDownloadWorker(downloadWorker);

	// if upload create uploadWorker
//		UploadWorker* uploadWorker = new UploadWorker();
//	thread_info* uploadWorkerThread = createUploadWorker(uploadWorker);

}

thread_info *NodeMessageListener::createDownloadWorker(DownloadWorker *pWorker) {
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

thread_info *NodeMessageListener::createUploadWorker(UploadWorker *pWorker) {
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
							reinterpret_cast<void *(*)(void *)>(&UploadWorker::start), &tinfo);
	if (status != 0)
		throw std::runtime_error("pthread_create");

	status = pthread_attr_destroy(&attr);
	if (status != 0)
		throw std::runtime_error("pthread_attr_destroy");


	return tinfo;
}
