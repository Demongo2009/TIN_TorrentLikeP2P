//
// Created by konrad on 5/30/21.
//

#ifndef TIN_TORRENTLIKEP2P_SERIALIZATIONUTILS_H
#define TIN_TORRENTLIKEP2P_SERIALIZATIONUTILS_H

#include <vector>
#include "ResourceInfo.h"

class SerializationUtils {
    char* serializeResourceInfo(ResourceInfo);
    char* serializeVectorOfResourceInfo(std::vector<ResourceInfo>); //zwraca zserialziowany caly wektor
};


#endif //TIN_TORRENTLIKEP2P_SERIALIZATIONUTILS_H
