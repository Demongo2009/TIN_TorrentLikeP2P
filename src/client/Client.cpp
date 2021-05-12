//
// Created by demongo on 11.05.2021.
//

#include <stdexcept>
#include "../../include/client/Client.h"
#include "../../include/Thread.h"
#include "../../include/client/CLI/CLI.h"
#include "../../include/client/serverCommunication/ServerListener.h"

void *Client::run(void *threadInfo) {
	int status;
	void* res;


	struct thread_info** threads;

	CLI* cli = new CLI();
	thread_info* cliThread = createCLI(cli);

	ServerListener* serverListener = new ServerListener();
	thread_info* serverListenerThread = createServerListener(serverListener);


	// can be done in loop with threads array
	status = pthread_join(cliThread->thread_id, &res);
	if(status != 0){
		throw std::runtime_error("pthread_join");
	}
	free(cliThread);

	status = pthread_join(serverListenerThread->thread_id, &res);
	if(status != 0){
		throw std::runtime_error("pthread_join");
	}
	free(serverListenerThread);

	free(res);

}

thread_info *Client::createCLI(CLI *pCli) {
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
	tinfo->context = pCli;
//	tinfo->string = workDir;

	status = pthread_create(&tinfo->thread_id, &attr,
							reinterpret_cast<void *(*)(void *)>(&CLI::start), &tinfo);
	if (status != 0)
		throw std::runtime_error("pthread_create");

	status = pthread_attr_destroy(&attr);
	if (status != 0)
		throw std::runtime_error("pthread_attr_destroy");


	return tinfo;
}

thread_info *Client::createServerListener(ServerListener *pListener) {
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
	tinfo->context = pListener;
//	tinfo->string = workDir;

	status = pthread_create(&tinfo->thread_id, &attr,
							reinterpret_cast<void *(*)(void *)>(&ServerListener::start), &tinfo);
	if (status != 0)
		throw std::runtime_error("pthread_create");

	status = pthread_attr_destroy(&attr);
	if (status != 0)
		throw std::runtime_error("pthread_attr_destroy");


	return tinfo;
}
