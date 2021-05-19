//
// Created by demongo on 11.05.2021.
//

#ifndef TIN_TORRENTLIKEP2P_NODEMESSAGELISTENER_H
#define TIN_TORRENTLIKEP2P_NODEMESSAGELISTENER_H


#include "../../Thread.h"
#include "../workerThreads/downloadWorker/DownloadWorker.h"
#include "../workerThreads/uploadWorker/UploadWorker.h"

class NodeMessageListener {

public:
	NodeMessageListener(){}

	void *run(void *threadInfo);

	static void *start(void*threadInfo){
		return ((NodeMessageListener*)(static_cast<thread_info*>(threadInfo)->context))->run(threadInfo);
	}

	thread_info *createDownloadWorker(DownloadWorker *pWorker);

	thread_info *createUploadWorker(UploadWorker *pWorker);
};


#endif //TIN_TORRENTLIKEP2P_NODEMESSAGELISTENER_H
