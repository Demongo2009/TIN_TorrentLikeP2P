#ifndef TIN_TORRENTLIKEP2P_MESSAGE_H
#define TIN_TORRENTLIKEP2P_MESSAGE_H

#define MAX_MESSAGE_SIZE 4096
#define HEADER_SIZE 4
#define MAX_SIZE_OF_PAYLOAD MAX_MESSAGE_SIZE-HEADER_SIZE
#define CHUNK_SIZE MAX_SIZE_OF_PAYLOAD-3

#include <vector>
#include <string>
#include <stdexcept>

//      NAZWA=KOD                       CO PRZESYLAMY
enum TcpMessageCode {
    DEMAND_CHUNK=140,                   // resourceName, indexOfChunk
    MY_STATE_BEFORE_FILE_TRANSFER=141,   // tablica krotek: (resourceName, revokePassword, sizeInBytes)
    CHUNK_TRANSFER=142,                 // indexOfChunk, offsetFromChunkStart, data
    SYNC_END = 143,
    REQUEST_SYNC_END,
    ERROR_AFTER_SYNCHRONIZATION=440,    // EMPTY
    ERROR_WHILE_SENDING=540,            // EMPTY
    ERROR_WHILE_RECEIVING=541,          // EMPTY
    INVALID_CHUNK_REQUEST
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
    std::vector<unsigned int> chunkIndices;
        //konstruktor domyslny
    DemandChunkMessage(std::string resourceName="",
                       std::vector<unsigned int> chunkIndices={}):
                            resourceName(std::move(resourceName)),
                            chunkIndices(std::move(chunkIndices)) {}

     static DemandChunkMessage deserializeChunkMessage(const char *message) {
         //zakladam, ze tutaj struktura jest "name;index1;index2;...indexn;000..."
         std::string name;
         std::vector<unsigned int> indices;

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
                     indices.push_back(std::stoi(currentIndex));
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
                 indices.push_back(std::stoi(currentIndex));
             }
             catch(std::exception& exception){
                 throw std::runtime_error("Exceeded uint limit or invalid character while reading index");
             }
         }
         return DemandChunkMessage(name,
                                   indices);
     }

};




#endif //TIN_TORRENTLIKEP2P_MESSAGE_H
