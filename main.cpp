#include <memory>
#include <mutex>
#include <csignal>
#include "include/threads/TorrentClient.h"


TorrentClient t;

void signalHandler(int signum){
    t.signalHandler();
}

int main(){

    std::signal(SIGTERM, signalHandler);
    std::signal(SIGQUIT, signalHandler);
    std::signal(SIGINT, signalHandler);
    t.run();

}