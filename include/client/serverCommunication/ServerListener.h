//
// Created by demongo on 11.05.2021.
//

#ifndef TIN_TORRENTLIKEP2P_SERVERLISTENER_H
#define TIN_TORRENTLIKEP2P_SERVERLISTENER_H


#include "../../Thread.h"

class ServerListener {

public:
	void *run(void *threadInfo);

	static void *start(void*threadInfo){
		return ((ServerListener*)(static_cast<thread_info*>(threadInfo)->context))->run(threadInfo);
	}
};


#endif //TIN_TORRENTLIKEP2P_SERVERLISTENER_H
