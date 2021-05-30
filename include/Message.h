#ifndef TIN_TORRENTLIKEP2P_MESSAGE_H
#define TIN_TORRENTLIKEP2P_MESSAGE_H

#define MAX_SIZE_OF_PAYLOAD 500


//      NAZWA=KOD                       CO PRZESYLAMY
enum TcpMessageCode {
    DEMAND_CHUNK=140,                   // resourceName, indexOfChunk
    MY_STATE_BEFORE_FILE_SENDING=141,   // tablica krotek: (resourceName, revokeHash, sizeInBytes)
    CHUNK_TRANSFER=142,                 // indexOfChunk, offsetFromChunkStart, data
    ERROR_AFTER_SYNCHRONIZATION=440,    // EMPTY
    ERROR_WHILE_SENDING=540,            // EMPTY
    ERROR_WHILE_RECEIVING=541,          // EMPTY

};
enum UdpMessageCode {
    NEW_RESOURCE_AVAILABLE=100,         // resourceName, revokeHash, sizeInBytes
    OWNER_REVOKED_RESOURCE=110,         // resourceName
    NODE_DELETED_RESOURCE=111,          // resourceName
    NEW_NODE_IN_NETWORK=120,            // EMPTY
    STATE_OF_NODE=121,                  // tablica krotek: (resourceName, revokeHash, sizeInBytes)
    NODE_LEFT_NETWORK=130,              // EMPTY - w sprawku napisalem ze przesyla to co miał, ale to w zasadzie niepotrzebne
};


/*
 *  TODO:
 *      1.nie wiem czy rozgraniczenie pomiedzy
 *      tcp i udp jest potrzebne, chyba nie, ale w ten sposób ładnie to wygląda.
 *      .
 *      2.podczas wysylania/odbierania kazdego komunikatu trzeba bedzie sprwdzac
 *      kod a potem w zalenosci od niego odpowienio serializowac/deserialziowac.
*/
 struct TcpMessage {
     //
    TcpMessageCode code;
    unsigned int sizeOfPayload;
    char payload[MAX_SIZE_OF_PAYLOAD];//tam gdzie mamy tablice w wysylanych komunikatach to pewnie trzeba jeszcze dodac liczbe przesylanych elementow na początku
};


#endif //TIN_TORRENTLIKEP2P_MESSAGE_H
