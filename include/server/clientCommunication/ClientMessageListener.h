//
// Created by demongo on 11.05.2021.
//

#ifndef TIN_TORRENTLIKEP2P_CLIENTMESSAGELISTENER_H
#define TIN_TORRENTLIKEP2P_CLIENTMESSAGELISTENER_H


#include <vector>
#include "../../Thread.h"
#include "../workerThreads/downloadWorker/DownloadWorker.h"



class ClientMessageListener {

public:
	ClientMessageListener(){}

	void *run(void *threadInfo);

	static void *start(void*threadInfo){
		return ((ClientMessageListener*)(static_cast<thread_info*>(threadInfo)->context))->run(threadInfo);
	}

	thread_info *createDownloadWorker(DownloadWorker *pWorker);

	void broadcastNewResource(void *pMetadata);

	void sendMessageToClient(std::vector<std::string> vector);
};


#endif //TIN_TORRENTLIKEP2P_CLIENTMESSAGELISTENER_H
