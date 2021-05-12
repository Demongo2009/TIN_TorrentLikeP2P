//
// Created by demongo on 11.05.2021.
//

#ifndef TIN_TORRENTLIKEP2P_CLIENT_H
#define TIN_TORRENTLIKEP2P_CLIENT_H


#include "../Thread.h"
#include "CLI/CLI.h"
#include "serverCommunication/ServerListener.h"

class Client {


public:
	Client(){}

	void *run(void *threadInfo);

	static void *start(void*threadInfo){
		return ((Client*)(static_cast<thread_info*>(threadInfo)->context))->run(threadInfo);
	}

	thread_info *createCLI(CLI *pCli);

	thread_info *createServerListener(ServerListener *pListener);
};


#endif //TIN_TORRENTLIKEP2P_CLIENT_H
