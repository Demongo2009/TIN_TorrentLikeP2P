#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <cstring>
#include "../../include/utils/utils.h"
void errno_abort(const std::string &header){
    perror(header.c_str());
}

std::pair<unsigned long, unsigned short> convertAddress(sockaddr_in address){
    return std::make_pair(address.sin_addr.s_addr, address.sin_port);
}

unsigned long convertAddressLong(sockaddr_in address){
    return address.sin_addr.s_addr;
}

std::string getMyAddress(int socket){
    struct ifaddrs *ifap, *ifa;
    std::string addr;
    std::string ethernetInterfaceName;
    std::string ethernetInterfaceNameBegin = "e";
    getifaddrs (&ifap);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family==AF_INET) {
            if(strncmp(ifa->ifa_name, ethernetInterfaceNameBegin.c_str(), 1) == 0){
                ethernetInterfaceName = ifa->ifa_name;
                break;
            }
        }
    }

    freeifaddrs(ifap);
    struct ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;
    memcpy(ifr.ifr_name, ethernetInterfaceName.c_str(), IFNAMSIZ-1);
    ioctl(socket, SIOCGIFADDR, &ifr);
    addr = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    return addr;
}

unsigned int getCountBytesToWrite(unsigned long long fileSize, unsigned long index, unsigned int chunkSize){
    unsigned long offset = index * chunkSize;
    if (offset + chunkSize <= fileSize) {
        return chunkSize;
    } else {
        return fileSize - offset;
    }
}