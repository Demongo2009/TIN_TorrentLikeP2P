#include <memory>
#include <csignal>
#include <iostream>
#include "include/threads/TorrentClient.h"

SharedStructs structs;
TorrentClient t = TorrentClient(structs);

void signalHandler(int signum){
    t.signalHandler();
}

int main(){


    std::signal(SIGTERM, signalHandler);
    std::signal(SIGQUIT, signalHandler);
    std::signal(SIGINT, signalHandler);
    t.run();

}