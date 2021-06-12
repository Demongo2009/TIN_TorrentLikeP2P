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

 struct DemandChunkMessage {
    std::string resourceName;
    std::vector<unsigned long> chunkIndices;

    explicit DemandChunkMessage(std::string resourceName="",
                       std::vector<unsigned long> chunkIndices={}):
                            resourceName(std::move(resourceName)),
                            chunkIndices(std::move(chunkIndices)) {}

     static DemandChunkMessage deserializeChunkMessage(const char *message);

};
struct ChunkTransfer{
    TcpMessageCode header;
    unsigned long index;
    char payload[CHUNK_SIZE + 1];
    ChunkTransfer( TcpMessageCode header, unsigned long index, const char* payload): header(header), index(index){
        strncpy(this->payload, payload, strlen(payload) );
        this->payload[strlen(payload)] = '\0';

    }
    static std::optional<ChunkTransfer> deserializeChunkTransfer(const char *message, unsigned long long fileSize);
};
#endif //TIN_TORRENTLIKEP2P_MESSAGE_H
