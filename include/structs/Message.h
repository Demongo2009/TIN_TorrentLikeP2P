#ifndef TIN_TORRENTLIKEP2P_MESSAGE_H
#define TIN_TORRENTLIKEP2P_MESSAGE_H

#define MAX_SIZE_OF_PAYLOAD 512
#define HEADER_SIZE 3
#define MAX_MESSAGE_SIZE HEADER_SIZE+MAX_SIZE_OF_PAYLOAD
#define CHUNK_SIZE MAX_SIZE_OF_PAYLOAD-3


//      NAZWA=KOD                       CO PRZESYLAMY
enum TcpMessageCode {
    DEMAND_CHUNK=140,                   // resourceName, indexOfChunk
    MY_STATE_BEFORE_FILE_TRANSFER=141,   // tablica krotek: (resourceName, revokePassword, sizeInBytes)
    CHUNK_TRANSFER=142,                 // indexOfChunk, offsetFromChunkStart, data
    ERROR_AFTER_SYNCHRONIZATION=440,    // EMPTY
    ERROR_WHILE_SENDING=540,            // EMPTY
    ERROR_WHILE_RECEIVING=541,          // EMPTY

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
};




#endif //TIN_TORRENTLIKEP2P_MESSAGE_H
