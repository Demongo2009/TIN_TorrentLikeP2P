//
// Created by bartlomiej on 03.06.2021.
//

#ifndef TIN_TORRENTLIKEP2P_UTILS_H
#define TIN_TORRENTLIKEP2P_UTILS_H

#include <string>
#include <netinet/in.h>

void errno_abort(const std::string &header);

std::pair<unsigned long, unsigned short> convertAddress(sockaddr_in address);

#endif //TIN_TORRENTLIKEP2P_UTILS_H
