//
// Created by demongo on 11.05.2021.
//

#ifndef TIN_TORRENTLIKEP2P_DOWNLOADWORKER_H
#define TIN_TORRENTLIKEP2P_DOWNLOADWORKER_H


#include "../../../Thread.h"

class DownloadWorker {

public:
	void *run(void *threadInfo);

	static void *start(void*threadInfo){
		return ((DownloadWorker*)(static_cast<thread_info*>(threadInfo)->context))->run(threadInfo);
	}
};


#endif //TIN_TORRENTLIKEP2P_DOWNLOADWORKER_H
