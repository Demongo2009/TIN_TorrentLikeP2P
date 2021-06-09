//
// Created by bartlomiej on 03.06.2021.
//

#ifndef TIN_TORRENTLIKEP2P_SHAREDSTRUCTS_H
#define TIN_TORRENTLIKEP2P_SHAREDSTRUCTS_H

#include <mutex>
#include <map>
#include "ResourceInfo.h"

struct SharedStructs{
    std::map<std::string , ResourceInfo> localResources;
    std::map<unsigned long,std::map<std::string, ResourceInfo> > networkResources;
    std::mutex localResourcesMutex;
    std::mutex networkResourcesMutex;
    std::map<std::string, std::string> filepaths;

};

#endif //TIN_TORRENTLIKEP2P_SHAREDSTRUCTS_H
