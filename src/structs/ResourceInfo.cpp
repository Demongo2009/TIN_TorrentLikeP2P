#include "../../include/structs/ResourceInfo.h"

ResourceInfo ResourceInfo::deserializeResource(const char *message, bool toVector,int *dataPointer) {
    std::string resourceName;
    unsigned long long sizeInBytes;
    std::size_t revokeHash;
    std::string builder;

    unsigned short charIndex=0;
    char currCharacter=message[charIndex];

    while(currCharacter && currCharacter!=SEPARATOR){
        resourceName+=currCharacter;
        currCharacter=message[++charIndex];
    }

    if(!currCharacter) {
        return ResourceInfo(resourceName);
    }
    currCharacter=message[++charIndex];
    std::string revokeHashBuilder;

    while(currCharacter && currCharacter!=SEPARATOR){
        if(isdigit(currCharacter)) {
            revokeHashBuilder += currCharacter;
        }else {
            throw std::runtime_error("invalid number while reading revoking hash (character is not a digit): ");
        }
        currCharacter=message[++charIndex];
    }

    try {
        std::stringstream ss(revokeHashBuilder);
        ss>>revokeHash;
    }
    catch (std::exception& exception){
        throw std::runtime_error("exceeded number value limit or invalid character read while reading resource revoke hash");
    }
    if(!currCharacter) {
        throw std::runtime_error("unexpected end of serialized data while reading resource name");
    }
    currCharacter=message[++charIndex];
    std::string sizeBuilder;

    while(currCharacter && currCharacter!=SEPARATOR){
        if(isdigit(currCharacter)) {
            sizeBuilder += currCharacter;
        }
        else {
            throw std::runtime_error("invalid number while reading resource size (character is not a digit)");
        }
        currCharacter=message[++charIndex];
    }
    try {
        sizeInBytes = std::stoull(sizeBuilder);
    }
    catch (std::exception& exception){
        throw std::runtime_error("exceeded number value limit or invalid character read while reading resource size");
    }


    if(toVector) {
        *dataPointer += charIndex;
    }
    if(charIndex > MAX_MESSAGE_SIZE) {
        throw std::runtime_error("message exceeded maximum length!");
    }

    return ResourceInfo(resourceName, sizeInBytes, revokeHash);
}

std::vector<ResourceInfo> ResourceInfo::deserializeVectorOfResources(char *message){
    int charIndex = 0;
    std::vector<ResourceInfo> resources;
    while (message[charIndex] && charIndex <= MAX_MESSAGE_SIZE){
        resources.push_back(std::move(deserializeResource(message + charIndex, true, &charIndex)));
        charIndex++;
    }

    if(charIndex > MAX_MESSAGE_SIZE) {
        throw std::runtime_error("message exceeded maximum length!");
    }

    return resources;
}