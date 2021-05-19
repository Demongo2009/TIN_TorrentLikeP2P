#include <iostream>
#include "../include/client/Client.h"
#include "../include/server/Server.h"
#include "../include/Thread.h"

struct thread_info* createServerThread(Server* server, char* workDir);

struct thread_info* createClientThread(Client* client);

int main(int argc, char *argv[]) {
	std::cout << "Hello, P2PWorld!" << std::endl;

	int status;
	// res is for result of start/run function, mostly unimportant for now
	void* res;

	Server* server = new Server();
	thread_info* serverThread = createServerThread(server,argv[1]);

	Client* client = new Client();
	thread_info* clientThread = createClientThread(client);

	status = pthread_join(clientThread->thread_id, &res);
	if(status != 0){
		throw std::runtime_error("pthread_join");
	}
	free(clientThread);

	status = pthread_join(serverThread->thread_id, &res);
	if(status != 0){
		throw std::runtime_error("pthread_join");
	}
	free(serverThread);
	free(res);

	return 0;
}

struct thread_info* createClientThread(Client* client) {
	pthread_attr_t attr;
	int status;

	status = pthread_attr_init(&attr);
	if(status != 0){
		throw std::runtime_error("pthread_attr_init!");
	}

	thread_info *tinfo = static_cast<thread_info *>(calloc(1, sizeof(*tinfo)));
	if (tinfo == NULL)
		throw std::runtime_error("calloc");

	///// thread_num for thread counting in debug dont know whether needed
	tinfo->thread_num = 2;
	tinfo->context = client;
//	tinfo->string = "";

	status = pthread_create(&tinfo->thread_id, &attr,
							reinterpret_cast<void *(*)(void *)>(&Client::start), &tinfo);
	if (status != 0)
		throw std::runtime_error("pthread_create");

	status = pthread_attr_destroy(&attr);
	if (status != 0)
		throw std::runtime_error("pthread_attr_destroy");


	return tinfo;
}

struct thread_info* createServerThread(Server* server, char * workDir) {
	pthread_attr_t attr;
	int status;

	status = pthread_attr_init(&attr);
	if(status != 0){
		throw std::runtime_error("pthread_attr_init!");
	}

	thread_info *tinfo = static_cast<thread_info *>(calloc(1, sizeof(*tinfo)));
	if (tinfo == NULL)
		throw std::runtime_error("calloc");

	tinfo->thread_num = 1;
	tinfo->string = workDir;
	tinfo->context = server;

	status = pthread_create(&tinfo->thread_id, &attr,
							reinterpret_cast<void *(*)(void *)>(&Server::start), &tinfo);
	if (status != 0)
		throw std::runtime_error("pthread_create");

	status = pthread_attr_destroy(&attr);
	if (status != 0)
		throw std::runtime_error("pthread_attr_destroy");


	return tinfo;
}
