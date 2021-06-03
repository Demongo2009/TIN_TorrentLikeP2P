#include <thread>
#include <iostream>
#include <sstream>
#include <fstream>
#include "../../include/threads/TorrentClient.h"

void TorrentClient::run() {

    std::thread udpThread = std::thread(&UdpThread::runUdpServerThread, udpObj);
    std::thread tcpThread = std::thread(&TcpThread::runTcpServerThread, tcpObj);
    std::thread cliThread = std::thread(&CliThread::runCliThread, cliObj);

    cliThread.join();
    tcpObj.terminate();
    udpObj.terminate();
    /**
     * 1.init struktur
     * 2.pthread_create(runServerThread)
     * 3.sleep(1sec)? - zeby poczekac chwilkę żebyśmy zdazyli sfetchowac stan sieci, mozemy to tez w wątku klienta przy czym wysietlimy jakas informacje typu "sekunda... inicjlaizacja węzła"
     * 4.phread_creat(runCliThread)
     */
}

void TorrentClient::signalHandler() {
    cliObj.terminate();
    tcpObj.terminate();
    udpObj.terminate();
}



