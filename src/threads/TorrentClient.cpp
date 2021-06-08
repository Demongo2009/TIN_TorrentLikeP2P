#include <thread>
#include <iostream>
#include <sstream>
#include <fstream>
#include "../../include/threads/TorrentClient.h"
#include <pthread.h>

void TorrentClient::run() {

	pthread_barrier_t barrier;
	const pthread_barrierattr_t *barrierattr;
	int success = pthread_barrier_init(&barrier, nullptr,3);
	udpObj->setBarrier(&barrier);
	tcpObj->setBarrier(&barrier);
	cliObj->setBarrier(&barrier);


    std::thread udpThread(UdpThread::start, udpObj.get());
    std::thread tcpThread(TcpThread::start, tcpObj.get());
    std::thread cliThread(CliThread::start, cliObj.get());
    cliThread.join();
    tcpObj->terminate();
    udpObj->terminate();


    /**
     * 1.init struktur
     * 2.pthread_create(runServerThread)
     * 3.sleep(1sec)? - zeby poczekac chwilkę żebyśmy zdazyli sfetchowac stan sieci, mozemy to tez w wątku klienta przy czym wysietlimy jakas informacje typu "sekunda... inicjlaizacja węzła"
     * 4.phread_creat(runCliThread)
     */
}

void TorrentClient::signalHandler() {
    cliObj->terminate();
    tcpObj->terminate();
    udpObj->terminate();
}



