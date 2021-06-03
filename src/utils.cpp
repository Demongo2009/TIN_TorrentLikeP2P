//
// Created by bartlomiej on 03.06.2021.
//

#include "../include/utils.h"
void errno_abort(const std::string &header){
    perror(header.c_str());
    exit(EXIT_FAILURE);
}

std::pair<unsigned long, unsigned short> convertAddress(sockaddr_in address){
    return std::make_pair(address.sin_addr.s_addr, address.sin_port);
}