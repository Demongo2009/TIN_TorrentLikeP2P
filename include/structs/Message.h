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

//      NAZWA=KOD                       CO PRZESYLAMY
enum TcpMessageCode {
    DEMAND_CHUNK=140,                   // resourceName, vectof<indexOfChunk>
    MY_STATE_BEFORE_FILE_TRANSFER=141,   // tablica krotek: (resourceName, revokePassword, sizeInBytes)
    CHUNK_TRANSFER=142,                 // indexOfChunk, data
    SYNC_END = 143,
    SYNC_OK = 144,
    CHUNK_TRANSFER_OK = 145,
    INVALID_CHUNK_REQUEST = 440
};
enum UdpMessageCode {
    NEW_RESOURCE_AVAILABLE=100,         // resourceName, revokePassword, sizeInBytes
    OWNER_REVOKED_RESOURCE=110,         // resourceName
    NODE_DELETED_RESOURCE=111,          // resourceName
    NEW_NODE_IN_NETWORK=120,            // EMPTY
    STATE_OF_NODE=121,                  // tablica krotek: (resourceName, revokePassword, sizeInBytes)
    NODE_LEFT_NETWORK=130,              // EMPTY - w sprawku napisalem ze przesyla to co miał, ale to w zasadzie niepotrzebne
};

enum ClientCommand {
    ADD_NEW_RESOURCE,
    LIST_AVAILABLE_RESOURCES,
    FIND_RESOURCE,
    DOWNLOAD_RESOURCE,
    REVOKE_RESOURCE,
    EXIT
};


/*
 *  TODO:
 *      1.nie wiem czy rozgraniczenie pomiedzy
 *      tcp i udp jest potrzebne, chyba nie, ale w ten sposób ładnie to wygląda.
 *      .
 *      2.podczas wysylania/odbierania kazdego komunikatu trzeba bedzie sprwdzac
 *      kod a potem w zalenosci od niego odpowienio serializowac/deserialziowac.
*/
 struct DemandChunkMessage {
    std::string resourceName;
    std::vector<unsigned long> chunkIndices;
        //konstruktor domyslny
    explicit DemandChunkMessage(std::string resourceName="",
                       std::vector<unsigned long> chunkIndices={}):
                            resourceName(std::move(resourceName)),
                            chunkIndices(std::move(chunkIndices)) {}

     static DemandChunkMessage deserializeChunkMessage(const char *message) {
         //zakladam, ze tutaj struktura jest "name;index1;index2;...indexn;000..."
         std::string name;
         std::vector<unsigned long> indices;

         unsigned short charIndex=0;
         char currCharacter=message[charIndex];

         while(currCharacter && currCharacter!=';'){
             name+=currCharacter;
             currCharacter=message[++charIndex];
         }
         //sprawdzanie czy nie ma konca pliku tam gdzie sie go nie spodziewamy
         if(!currCharacter)
             throw std::runtime_error("unexpected end of serialized data while reading chunk message name");
         //przechodzimy przez srednik
         currCharacter=message[++charIndex];

         std::string currentIndex;

         while(currCharacter){
             if(isdigit(currCharacter)){
                 currentIndex+=currCharacter;
             } else if(currCharacter==';'){
                 //mamy nowy index - wrzucamy do wektora i resetujemy stringa zbierajacego cyfry
                 try {
                     indices.push_back(std::stoul(currentIndex));
                 }
                 catch(std::exception& exception){
                     throw std::runtime_error("Exceeded uint limit or invalid character while reading index");
                 }
                 currentIndex="";
             } else{
                 //ani nie cyfra ani nie srednik
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
         return DemandChunkMessage(name,
                                   indices);
     }

};
struct ChunkTransfer{
    TcpMessageCode header;
    unsigned long index;
    char payload[CHUNK_SIZE + 1];
    ChunkTransfer( TcpMessageCode header, unsigned long index, char* payload, int payloadSize): header(header), index(index){
        std::cout<< "przed memcpy:"<<std::endl;
        memcpy(this->payload, payload, payloadSize);
        std::cout<< "po memcpy:"<<std::endl;
//        strncpy(this->payload, payload, strlen(payload) );
//        this->payload[payloadSize] = '\0';
        std::cout<< "po nullc:"<<std::endl;
    }
    static std::optional<ChunkTransfer> deserializeChunkTransfer(const char *message, unsigned long long fileSize) {
        TcpMessageCode header;
        unsigned long index;
        char payload[(CHUNK_SIZE) + 1];
        memset(payload, 0, sizeof payload);
        unsigned short charIndex=0;
        char currCharacter=message[charIndex];


        std::string currentIndex, headerStr;
        while (isdigit(currCharacter)){
            headerStr+=currCharacter;
            currCharacter=message[++charIndex];
        }
        if( std::stoi(headerStr) != CHUNK_TRANSFER )
            return std::nullopt;
        currCharacter=message[++charIndex];
        while (isdigit(currCharacter)){
            currentIndex+=currCharacter;
            currCharacter=message[++charIndex];
        }
        ++charIndex;
        std::cout<< "w deserialize charindex:"<<charIndex<<"message"<<message<<std::endl;
        int payloadSize, c = CHUNK_SIZE, m = MAX_MESSAGE_SIZE;
        long offset = index * c;
        if (offset + c <= fileSize) {
            payloadSize = c;
        } else {
            payloadSize = fileSize - offset;
        }
        memcpy( payload, message + charIndex, payloadSize);
//        payload[strlen(message) - charIndex] = '\0';
        std::cout<< "w deserialize przed return charindex:"<<charIndex<<"message"<<message<<std::endl;
        std::cout<< "w deserialize payloadsize:"<<payloadSize<<" payload:"<<payload<<std::endl;
        return ChunkTransfer((TcpMessageCode)std::stoi(headerStr), std::stoul(currentIndex), payload, payloadSize);
    }
};



#endif //TIN_TORRENTLIKEP2P_MESSAGE_H
