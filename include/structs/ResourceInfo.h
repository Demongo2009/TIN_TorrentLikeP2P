#ifndef TIN_TORRENTLIKEP2P_RESOURCEINFO_H
#define TIN_TORRENTLIKEP2P_RESOURCEINFO_H

#include <string>
#include <stdexcept>
#include "Message.h"
#include <sstream>
struct ResourceInfo {
    std::string resourceName;
    unsigned long long sizeInBytes;
    std::size_t revokeHash;
    bool isRevoked;
    explicit ResourceInfo(std::string resourceName="",
                 unsigned long long sizeInBytes=0,
                 std::size_t revokeHash=0,
                 bool isRevoked = false):
                    resourceName(std::move(resourceName)),
                    sizeInBytes(sizeInBytes),
                    revokeHash(revokeHash),
                    isRevoked(isRevoked) {}

    static ResourceInfo deserializeResource(const char *message, bool toVector = false,int *dataPointer = nullptr) {
        std::string resourceName;
        unsigned long long sizeInBytes;
        std::size_t revokeHash;

        std::string builder;

        unsigned short charIndex=0;
        char currCharacter=message[charIndex];

        while(currCharacter && currCharacter!=';'){
            resourceName+=currCharacter;
            currCharacter=message[++charIndex];
        }

        //sprawdzanie czy nie ma konca pliku tam gdzie sie go nie spodziewamy
        if(!currCharacter)
            return ResourceInfo(resourceName);

		currCharacter=message[++charIndex];
		std::string revokeHashBuilder;

		//do wektora to czekamy na albo NULL albo na srednik (;)
		while(currCharacter && currCharacter!=';'){
			if(isdigit(currCharacter))
				revokeHashBuilder+=currCharacter;
			else
				throw std::runtime_error("invalid number while reading revoking hash (character is not a digit): ");
			currCharacter=message[++charIndex];
		}

		try {
			std::stringstream ss(revokeHashBuilder);
			ss>>revokeHash;
		}
		catch (std::exception& exception){
			throw std::runtime_error("exceeded number value limit or invalid character read while reading resource revoke hash");
		}
		if(!currCharacter)
			throw std::runtime_error("unexpected end of serialized data while reading resource name");
        currCharacter=message[++charIndex];
        std::string sizeBuilder;

        while(currCharacter && currCharacter!=';'){
            if(isdigit(currCharacter))
                sizeBuilder+=currCharacter;
            else
                throw std::runtime_error("invalid number while reading resource size (character is not a digit)");
            currCharacter=message[++charIndex];
        }
        try {
            sizeInBytes = std::stoull(sizeBuilder);
        }
        catch (std::exception& exception){
            throw std::runtime_error("exceeded number value limit or invalid character read while reading resource size");
        }


        if(toVector)
            *dataPointer+=charIndex;

        if(charIndex > MAX_MESSAGE_SIZE)
            throw std::runtime_error("message exceeded maximum lenght!");

        return ResourceInfo(resourceName,
                            sizeInBytes,
                            revokeHash);
    }

    static std::vector<ResourceInfo> deserializeVectorOfResources(char *message)
    {
        int charIndex = 0;
        std::vector<ResourceInfo> resources;
        while (message[charIndex] && charIndex <= MAX_MESSAGE_SIZE)
        {
            resources.push_back(std::move(deserializeResource(message + charIndex, true, &charIndex)));
            charIndex++;
        }

        // troche pozno jest kiedy to pisze ale czy powinienem sprawdzac dlugosc tego, co dostaje?
//    czy to nie jest zapewnione w zaden sposob wyzej? Jezeli tak to najmocniej przepraszam, mozna to wywalic
        if(charIndex > MAX_MESSAGE_SIZE)
            throw std::runtime_error("message exceeded maximum lenght!");

        return resources;
    }

};


#endif //TIN_TORRENTLIKEP2P_RESOURCEINFO_H
