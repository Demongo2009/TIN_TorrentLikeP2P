#include <thread>
#include <fstream>
#include "../../include/threads/TorrentClient.h"
#include <pthread.h>

void TorrentClient::run() {

	pthread_barrier_t barrier;
	pthread_barrier_init(&barrier, nullptr,3);
	udpObj->setBarrier(&barrier);
	tcpObj->setBarrier(&barrier);
	cliObj->setBarrier(&barrier);


    std::thread udpThread(UdpThread::start, udpObj.get());
    std::thread tcpThread(TcpThread::start, tcpObj.get());
    std::thread cliThread(CliThread::start, cliObj.get());
    cliThread.join();
    tcpObj->terminate();
    udpObj->terminate();

}

void TorrentClient::signalHandler() {
    cliObj->terminate();
    tcpObj->terminate();
    udpObj->terminate();
}



