#ifndef TIN_TORRENTLIKEP2P_MESSAGE_H
#define TIN_TORRENTLIKEP2P_MESSAGE_H

#define MAX_MESSAGE_SIZE 1024
#define HEADER_SIZE 4
#define MAX_SIZE_OF_PAYLOAD (MAX_MESSAGE_SIZE-HEADER_SIZE)
#define CHUNK_SIZE (MAX_SIZE_OF_PAYLOAD-16)

#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <optional>

const char SEPARATOR = ';';

enum TcpMessageCode {
    DEMAND_CHUNK=140,
    MY_STATE_BEFORE_FILE_TRANSFER=141,
    CHUNK_TRANSFER=142,
    SYNC_END = 143,
    SYNC_OK = 144,
    CHUNK_TRANSFER_OK = 145,
    INVALID_CHUNK_REQUEST = 440
};
enum UdpMessageCode {
    NEW_RESOURCE_AVAILABLE=100,
    OWNER_REVOKED_RESOURCE=110,
    NODE_DELETED_RESOURCE=111,
    NEW_NODE_IN_NETWORK=120,
    STATE_OF_NODE=121,
    NODE_LEFT_NETWORK=130,
};

enum ClientCommand {
    ADD_NEW_RESOURCE,
    LIST_AVAILABLE_RESOURCES,
    FIND_RESOURCE,
    DOWNLOAD_RESOURCE,
    REVOKE_RESOURCE,
    EXIT,
    UNKNOWN
};




 struct DemandChunkMessage {
    std::string resourceName;
    std::vector<unsigned long> chunkIndices;

    explicit DemandChunkMessage(std::string resourceName="",
                       std::vector<unsigned long> chunkIndices={}):
                            resourceName(std::move(resourceName)),
                            chunkIndices(std::move(chunkIndices)) {}

     static DemandChunkMessage deserializeChunkMessage(const char *message) {
         std::string name;
         std::vector<unsigned long> indices;
         unsigned short charIndex=0;
         char currCharacter=message[charIndex];

         while(currCharacter && currCharacter!=SEPARATOR){
             name+=currCharacter;
             currCharacter=message[++charIndex];
         }

         if(!currCharacter) {
             throw std::runtime_error("unexpected end of serialized data while reading chunk message name");
         }
         currCharacter=message[++charIndex];

         std::string currentIndex;

         while(currCharacter){
             if(isdigit(currCharacter)){
                 currentIndex+=currCharacter;
             } else if(currCharacter==SEPARATOR){
                 try {
                     indices.push_back(std::stoul(currentIndex));
                 }
                 catch(std::exception& exception){
                     throw std::runtime_error("Exceeded uint limit or invalid character while reading index");
                 }
                 currentIndex="";
             } else{
                 throw std::runtime_error("Unexpected character while reading chunk message indices");
             }
             currCharacter=message[++charIndex];
         }
         if(!std::empty(currentIndex)){
             try {
                 indices.push_back(std::stoul(currentIndex));
             }
             catch(std::exception& exception){
                 throw std::runtime_error("Exceeded uint limit or invalid character while reading index");
             }
         }
         return DemandChunkMessage(name, indices);
     }

};
struct ChunkTransfer{
    TcpMessageCode header;
    unsigned long index;
    char payload[CHUNK_SIZE + 1];
    ChunkTransfer( TcpMessageCode header, unsigned long index, const char* payload): header(header), index(index){
        strncpy(this->payload, payload, strlen(payload) );
        this->payload[strlen(payload)] = '\0';

    }
    static std::optional<ChunkTransfer> deserializeChunkTransfer(const char *message, unsigned long long fileSize) {
        char payload[(CHUNK_SIZE) + 1];
        memset(payload, 0, sizeof payload);
        unsigned short charIndex=0;
        char currCharacter=message[charIndex];


        std::string currentIndex, headerStr;
        while (isdigit(currCharacter)){
            headerStr+=currCharacter;
            currCharacter=message[++charIndex];
        }
        if( std::stoi(headerStr) != CHUNK_TRANSFER ) {
            return std::nullopt;
        }
        currCharacter=message[++charIndex];
        while (isdigit(currCharacter)){
            currentIndex+=currCharacter;
            currCharacter=message[++charIndex];
        }
        ++charIndex;
        strncpy( payload, message + charIndex, strlen(message) - charIndex );
        payload[strlen(message) - charIndex] = '\0';
        return ChunkTransfer((TcpMessageCode)std::stoi(headerStr), std::stoul(currentIndex), payload);
    }
};
#endif //TIN_TORRENTLIKEP2P_MESSAGE_H
