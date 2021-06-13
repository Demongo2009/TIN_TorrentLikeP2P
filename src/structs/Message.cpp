#include "../../include/structs/Message.h"
#include "../../include/utils/utils.h"


DemandChunkMessage DemandChunkMessage::deserializeChunkMessage(const char *message) {
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

std::optional<ChunkTransfer> ChunkTransfer::deserializeChunkTransfer(const uint8_t *message, unsigned long long fileSize) {
    uint8_t payload[(CHUNK_SIZE) + 1];
    memset(payload, 0, sizeof payload);
    unsigned short charIndex=0;
    char currCharacter=(char)message[charIndex];


    std::string currentIndex, headerStr;
    while (isdigit(currCharacter)){
        headerStr+=currCharacter;
        currCharacter=(char)message[++charIndex];
    }
    if( std::stoi(headerStr) != CHUNK_TRANSFER ) {
        return std::nullopt;
    }
    currCharacter=(char)message[++charIndex];
    while (isdigit(currCharacter)){
        currentIndex+=currCharacter;
        currCharacter=(char)message[++charIndex];
    }
    ++charIndex;
    unsigned  int countBytesToWrite = getCountBytesToWrite(fileSize, std::stoul(currentIndex), CHUNK_SIZE );
//    strncpy( payload, message + charIndex, strlen(message) - charIndex );
//    payload[strlen(message) - charIndex] = '\0';
    memcpy(payload, message + charIndex, countBytesToWrite );
    return ChunkTransfer((TcpMessageCode)std::stoi(headerStr), std::stoul(currentIndex), payload, countBytesToWrite);
}
