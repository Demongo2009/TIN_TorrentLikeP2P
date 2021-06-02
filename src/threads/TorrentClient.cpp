#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#include "../../include/threads/TorrentClient.h"


void TorrentClient::run() {
    std::thread serverUdpThread(&TorrentClient::runUdpServerThread, this);
    std::thread serverTcpThread(&TorrentClient::runTcpServerThread, this);
    std::thread cliThread(&TorrentClient::runCliThread, this);

    cliThread.join();
    /**
     * 1.init struktur
     * 2.pthread_create(runServerThread)
     * 3.sleep(1sec)? - zeby poczekac chwilkę żebyśmy zdazyli sfetchowac stan sieci, mozemy to tez w wątku klienta przy czym wysietlimy jakas informacje typu "sekunda... inicjlaizacja węzła"
     * 4.phread_creat(runCliThread)
     */
}

void TorrentClient::handleExit() {
    close(broadcastSocket);
}

void TorrentClient::signalHandler() {
    keepGoing = false;
    handleExit();
}

void TorrentClient::receive(int socket, bool tcp){
    char rbuf[MAX_MESSAGE_SIZE];
    memset(rbuf, 0, MAX_MESSAGE_SIZE);
    struct sockaddr_in clientAddr;
    socklen_t clientLength = sizeof(sockaddr_in);
    if (recvfrom(socket, rbuf, sizeof(rbuf) - 1, 0,(struct sockaddr *) &clientAddr, &clientLength) < 0) {
        perror("receive error");
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    printf("recv: %s\n", rbuf);
#endif
    char header[HEADER_SIZE];
    char payload[MAX_SIZE_OF_PAYLOAD];
    memset(header, 0, HEADER_SIZE);
    memset(payload, 0, MAX_SIZE_OF_PAYLOAD);
    snprintf(header, sizeof(header), "%s", rbuf);
    snprintf(payload, sizeof(payload), "%s", rbuf+HEADER_SIZE+1);
    if(tcp){
        handleTcpMessage(header, payload, socket);
    }else{
        handleUdpMessage(header, payload, clientAddr);
    }

}

/**
 *
 * TCP BLOCK
 *
 */
void TorrentClient::initTcp(){
    struct sockaddr_in serverAddr;

    tcpSocket = socket(PF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;

    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(address.c_str());


    memset(serverAddr.sin_zero, 0, sizeof serverAddr.sin_zero);

    int returnCode = bind(tcpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    if (returnCode == -1) {
        printf("BIND ERROR: %s\n", strerror(errno));
        close(tcpSocket);
        exit(1);
    }

    returnCode = listen(tcpSocket, 16);
    if (returnCode == -1){
        printf("Listen ERROR: %s\n", strerror(errno));
        close(tcpSocket);
        exit(1);
    }
    std::cout<<"Listening"<<std::endl;
}

int TorrentClient::acceptClient() {
    printf("waiting for client...\n");
    struct sockaddr_in clientAddr;
    socklen_t size = sizeof(clientAddr);
    int clientSocket = accept(tcpSocket, (struct sockaddr *) &clientAddr, &size);
    if (clientSocket == -1){
        printf("ACCEPT ERROR: %s\n", strerror(errno));
    }

    int currentReturnCode = getpeername(clientSocket, (struct sockaddr *) &clientAddr, &size);
    if (currentReturnCode == -1){
        printf("GETPEERNAME ERROR: %s\n", strerror(errno));
    }
    else {
        std::cout<<"Client address: "<< inet_ntoa(clientAddr.sin_addr);
    }
    connectedClients.emplace_back(clientSocket);
    return clientSocket;
}

[[noreturn]] void TorrentClient::runTcpServerThread() {

    initTcp();
    while (true) {
        receive(acceptClient(), true);
    }
}


void TorrentClient::handleTcpMessage(char *header, char *payload, int socket) {
    switch (atoi(header)) {
        case DEMAND_CHUNK:
            handleDemandChunk(payload, socket);
            break;
        case MY_STATE_BEFORE_FILE_TRANSFER:
            handleMyStateBeforeFileTransfer(payload, socket);
            break;
        case CHUNK_TRANSFER:
            handleChunkTransfer(payload, socket);
            break;
        case ERROR_AFTER_SYNCHRONIZATION:
            handleErrorAfterSynchronization(payload, socket);
            break;
        case ERROR_WHILE_RECEIVING:
            handleErrorWhileReceiving(payload, socket);
            break;
        case ERROR_WHILE_SENDING:
            handleErrorWhileSending(payload, socket);
            break;
    }
}


// sending convention HEADER , {';' , MESSAGE_ELEMENT};

void TorrentClient::handleDemandChunk(char *payload, int socket) {
    std::thread demandThread(&TorrentClient::demandChunkJob, this, payload, socket);
}

void TorrentClient::demandChunkJob(char *payload, int socket){
    DemandChunkMessage message = deserializeChunkMessage(payload);
    std::string filepath = filepaths.at(message.resourceName);
    std::ifstream ifs {filepath, std::ios::in | std::ios_base::binary};
    localResourcesMutex.lock();
    long fileSize = localResources_.at(message.resourceName).sizeInBytes;
    localResourcesMutex.unlock();
    char chunk[CHUNK_SIZE];
    char sbuf[HEADER_SIZE + MAX_SIZE_OF_PAYLOAD] = {};
    for(const auto & index : message.chunkIndices) {
        long offset = index * CHUNK_SIZE;
        ifs.seekg(offset, std::ios::beg);
        memset(chunk, 0, CHUNK_SIZE);
        if (offset + CHUNK_SIZE <= fileSize) {
            ifs.read(chunk, CHUNK_SIZE);
        } else if (offset > fileSize) {
            ifs.read(chunk, fileSize - offset);
        } else {
            std::cout << "INVALID CHUNK DEMAND" << std::endl;
        }

        memset(sbuf, 0, sizeof(sbuf));
        snprintf(sbuf, sizeof(sbuf), "%d;%d;%s", CHUNK_TRANSFER, index, chunk);

        if (send(socket, sbuf, sizeof sbuf, 0)) {
            errno_abort("send chunk");
        }
    }
    close(socket);

}

void TorrentClient::handleMyStateBeforeFileTransfer(char *payload, int socket) {
    std::thread findThread(&TorrentClient::stateBeforeFileTransferJob, this, payload, socket);
}

void TorrentClient::stateBeforeFileTransferJob(char *payload, int socket){

}

void TorrentClient::handleChunkTransfer(char *payload, int socket) {
    std::thread findThread(&TorrentClient::chunkTransferJob, this, payload, socket);
}

void TorrentClient::chunkTransferJob(char *payload, int socket){

}

void TorrentClient::handleErrorAfterSynchronization(char *payload, int socket) { //todo z rozpędu ale pewnie bez nowych nitek te errorhandlery
    std::thread findThread(&TorrentClient::errorAfterSyncJob, this, payload, socket);
}

void TorrentClient::errorAfterSyncJob(char *payload, int socket){

}

void TorrentClient::handleErrorWhileReceiving(char *payload, int socket) {
    std::thread findThread(&TorrentClient::errorWhileReceivingJob, this, payload, socket);
}

void TorrentClient::errorWhileReceivingJob(char *payload, int socket){

}

void TorrentClient::handleErrorWhileSending(char *payload, int socket) {
    std::thread findThread(&TorrentClient::errorWhileSendingJob , this, payload, socket);
}

void TorrentClient::errorWhileSendingJob(char *payload, int socket){

}


/**
 *
 * UDP BLOCK
 *
 */

void TorrentClient::handleUdpMessage(char *header, char *payload, sockaddr_in sockaddr) {
    switch (atoi(header)) {
        case NEW_RESOURCE_AVAILABLE:
            handleNewResourceAvailable(payload, sockaddr);
            break;
        case OWNER_REVOKED_RESOURCE:
            handleOwnerRevokedResource(payload, sockaddr);
            break;
        case NODE_DELETED_RESOURCE:
            handleNodeDeletedResource(payload, sockaddr);
            break;
        case NEW_NODE_IN_NETWORK:
            handleNewNodeInNetwork(payload, sockaddr);
            break;
        case STATE_OF_NODE:
            handleStateOfNode(payload, sockaddr);
            break;
        case NODE_LEFT_NETWORK:
            handleNodeLeftNetwork(payload, sockaddr);
            break;
    }
}



[[noreturn]] void TorrentClient::runUdpServerThread() {
    initUdp();
    broadcastNewNode();
    while (true){
        receive(udpSocket, false);
    }
}

void TorrentClient::initUdp() {
    struct sockaddr_in recv_addr;
    int trueFlag = 1;
    if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        errno_abort("socket");
    }
    if (setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof trueFlag) < 0) {
        errno_abort("setsockopt");
    }

    memset(&recv_addr, 0, sizeof recv_addr);
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_port = (in_port_t) htons(port);
    recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(udpSocket, (struct sockaddr*) &recv_addr, sizeof recv_addr) < 0) {
        errno_abort("bind");
    }


    if((broadcastSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        errno_abort("socket");
    }

    if (setsockopt(broadcastSocket, SOL_SOCKET, SO_BROADCAST, &trueFlag, sizeof trueFlag) < 0) {
        errno_abort("setsockopt");
    }

    memset(&broadcastAddress, 0, sizeof broadcastAddress);
    broadcastAddress.sin_family = AF_INET;
    broadcastAddress.sin_port = (in_port_t) htons(port);
    // broadcasting address for unix (?)
    inet_aton("127.255.255.255", &broadcastAddress.sin_addr);

}

void TorrentClient::genericBroadcast(UdpMessageCode code, char *payload) const {

    char sbuf[HEADER_SIZE + MAX_SIZE_OF_PAYLOAD] = {};
    if( strlen(payload) > MAX_SIZE_OF_PAYLOAD ){
    	std::cout << "The payload is too big!\n";
		return;
    }
    snprintf(sbuf, sizeof(sbuf), "%d;%s", code, payload);

    if (sendto(broadcastSocket, sbuf, strlen(sbuf) + 1, 0, (struct sockaddr *) &broadcastAddress, sizeof broadcastAddress) < 0) {
        errno_abort("send");
    }

#ifdef DEBUG
    printf("send new node: %s\n", sbuf);
#endif

}


void TorrentClient::broadcastNewNode(){
    char* buf = {};
    genericBroadcast(NEW_NODE_IN_NETWORK, buf);
}

void TorrentClient::broadcastNewFile(const ResourceInfo& resource)
{
    char sbuf[MAX_SIZE_OF_PAYLOAD] = {};
    snprintf(sbuf, sizeof(sbuf),
             "%s;%lu;%d",
             resource.resourceName.c_str(),
             resource.revokeHash,
             resource.sizeInBytes);

    genericBroadcast(NEW_RESOURCE_AVAILABLE, sbuf);
}

void TorrentClient::broadcastRevokeFile(const ResourceInfo& resource){
    char sbuf[MAX_SIZE_OF_PAYLOAD] = {};
    snprintf(sbuf, sizeof(sbuf), "%s", resource.resourceName.c_str());
    genericBroadcast(OWNER_REVOKED_RESOURCE, sbuf);
}

void TorrentClient::broadcastFileDeleted(const ResourceInfo& resource){
    char sbuf[MAX_SIZE_OF_PAYLOAD] = {};
    snprintf(sbuf, sizeof(sbuf), "%s", resource.resourceName.c_str());
    genericBroadcast(NODE_DELETED_RESOURCE, sbuf);
}

void TorrentClient::broadcastLogout(const std::vector<ResourceInfo>& resources){
    std::stringstream ss;
	char sbuf[MAX_SIZE_OF_PAYLOAD] = {};

	for(const auto& resource: resources){
    	if(ss.str().size() + resource.resourceName.size() > MAX_SIZE_OF_PAYLOAD){ //todo chyba niepotrzebne to wysyłać
			snprintf(sbuf, sizeof(sbuf), "%s", ss.str().c_str());
			genericBroadcast(NODE_LEFT_NETWORK, sbuf);
			ss.clear();
    	}
        ss << ";" << resource.resourceName;
    }
	snprintf(sbuf, sizeof(sbuf), "%s", ss.str().c_str());
	genericBroadcast(NODE_LEFT_NETWORK, sbuf);
}


//te funkcje handlujące nie tworzą nowych nitek deserializacja i aktualizacja struktur
void TorrentClient::handleNewResourceAvailable(char *message, sockaddr_in sockaddr) {
    ResourceInfo resource = deserializeResource(message);
    networkResourcesMutex.lock();
    networkResources_[convertAddress(sockaddr)][resource.resourceName] = resource;
    networkResourcesMutex.unlock();

}

void TorrentClient::handleOwnerRevokedResource(char *message, sockaddr_in sockaddr) {
    ResourceInfo resource = deserializeResource(message);
    networkResourcesMutex.lock();
    networkResources_[convertAddress(sockaddr)][resource.resourceName].isRevoked = true;
//    networkResources_[convertAddress(sockaddr)].erase(resource.resourceName); ???
    networkResourcesMutex.unlock();
}

void TorrentClient::handleNodeDeletedResource(char *message, sockaddr_in sockaddr) {
    ResourceInfo resource = deserializeResource(message);
    networkResourcesMutex.lock();
    networkResources_[convertAddress(sockaddr)].erase(resource.resourceName);
    networkResourcesMutex.unlock();
}

void TorrentClient::handleNewNodeInNetwork(char *message, sockaddr_in sockaddr) {
    networkResourcesMutex.lock();
    networkResources_.insert(std::make_pair(convertAddress(sockaddr), std::map<std::string, ResourceInfo>()));
    networkResourcesMutex.unlock();
    sendMyState(sockaddr);
}



void TorrentClient::handleStateOfNode(char *message, sockaddr_in sockaddr) {
    std::vector<ResourceInfo> resources = {};//deserialize(message);
    networkResourcesMutex.lock();
    for(const auto & r : resources){
        networkResources_[convertAddress(sockaddr)][r.resourceName] = r;
    }
    networkResourcesMutex.unlock();
}

void TorrentClient::handleNodeLeftNetwork(char *message, sockaddr_in sockaddr) {
    networkResourcesMutex.lock();
    networkResources_.erase(convertAddress(sockaddr));
    networkResourcesMutex.unlock();
}

/**
 *
 * CLI BLOCK
 *
 */


void TorrentClient::runCliThread() {
    //KUBA SPARSUJ
    // k

	std::stringstream ss;
	std::string line;
	std::string prompt = "\nPlease input command:\n"
						 "new <filePath> <resourceName>\n"
						 "list\n"
						 "find <resourceName>\n"
						 "download <resourceName>\n"
						 "revoke <resourceName>\n"
						 "q (in order to quit)\n"
						 "Please input resourceNames shorter than " +  std::to_string(MAX_FILE_NAME_SIZE) + "\n";
	std::cout << prompt;
	std::getline(std::cin, line);
	std::vector<std::string> vecWord;

    while(keepGoing){
        ClientCommand parsedCommand;
        std::string userString, resourceName;

		ss << line;
		for(std::string s; ss >>s;){
			vecWord.push_back(s);
		}

		bool foundCommand= true;
		parsedCommand = parseCommand(vecWord, userString, resourceName, foundCommand);

		if(foundCommand){
			switch (parsedCommand) {
				case ADD_NEW_RESOURCE:
					handleClientAddResource(resourceName, userString);
					break;
				case LIST_AVAILABLE_RESOURCES:
					handleClientListResources();
					break;
				case FIND_RESOURCE:
					handleClientFindResource(resourceName);
					break;
				case DOWNLOAD_RESOURCE:
					handleDownloadResource(resourceName);
					break;
				case REVOKE_RESOURCE:
					handleRevokeResource(resourceName, userString);
					break;
				case EXIT:
					handleExit();
					keepGoing = false;
					break;
			}
		}

		ss.clear();
		vecWord.clear();
		std::cout << prompt;
		std::getline(std::cin, line);
    }

    // clean before quit?
}

ClientCommand TorrentClient::parseCommand(std::vector<std::string> vecWord, std::string &filepath,
										  std::string &resourceName, bool &foundCommand){
	ClientCommand parsedCommand = EXIT;
	if(vecWord[0] == "new") {
		parsedCommand = ADD_NEW_RESOURCE;

		if(vecWord.size() > 1){
			filepath = vecWord[1];
		}else{
			std::cout << "You must input file path!\n";
			foundCommand = false;
			return parsedCommand;
		}

		if(vecWord.size() > 2){
			resourceName = vecWord[2];
		}else{
			std::cout << "You must input file name!\n";
			foundCommand = false;
			return parsedCommand;
		}

		if(resourceName.size() > MAX_FILE_NAME_SIZE){
			std::cout << "File name too long! Has: " << resourceName.size() << "\n";
			foundCommand = false;
			return parsedCommand;
		}

	} else if(vecWord[0] == "list"){
		parsedCommand = LIST_AVAILABLE_RESOURCES;

	} else if(vecWord[0] == "find"){
		parsedCommand = FIND_RESOURCE;

		parseResourceName(vecWord, resourceName, foundCommand);


	} else if(vecWord[0] == "download"){
		parsedCommand = DOWNLOAD_RESOURCE;

		parseResourceName(vecWord, resourceName, foundCommand);


	} else if(vecWord[0] == "revoke"){
		parsedCommand = REVOKE_RESOURCE;

		parseResourceName(vecWord, resourceName, foundCommand);

	} else if(vecWord[0] == "q"){
		parsedCommand = EXIT;

	}else{
		std::cout << "Unrecognised command!\n";
		foundCommand = false;
	}

	return parsedCommand;
}

void TorrentClient::parseResourceName(std::vector<std::string> vecWord, std::string &resourceName, bool& foundCommand){
	if(vecWord.size() > 1){
		resourceName = vecWord[1];
	}else{
		std::cout << "You must input file name!\n";
		foundCommand = false;
		return;
	}

	if(resourceName.size() > MAX_FILE_NAME_SIZE){
		std::cout << "File name too long! Has: " << resourceName.size() << "\n";
		foundCommand = false;
	}
}

//te nowe nitki robią
void TorrentClient::handleClientAddResource(const std::string& resourceName, const std::string& filepath) {

}

void TorrentClient::handleClientListResources() {
    std::thread findThread(&TorrentClient::listResourcesJob, this);
}

void TorrentClient::listResourcesJob(){

}

void TorrentClient::handleClientFindResource(const std::string& resourceName) {
    std::thread findThread(&TorrentClient::findResourceJob, this, resourceName);
}

void TorrentClient::findResourceJob(const std::string& resource){

}


void TorrentClient::handleDownloadResource(const std::string& resourceName) {
    std::thread findThread(&TorrentClient::downloadResourceJob, this, resourceName);
}

void TorrentClient::downloadResourceJob(const std::string& resource){

}


void TorrentClient::handleRevokeResource(const std::string& resourceName, const std::string& userPassword) {
    localResourcesMutex.lock();
    std::size_t hash = std::hash<std::string>{}(userPassword);
    if(localResources_.at(resourceName).revokeHash != hash ){
        std::cout<<"YOU HAVE NO RIGHT SIR"<<std::endl;
        return;
    }
    localResources_.at(resourceName).isRevoked = true;
    localResourcesMutex.unlock();
    broadcastRevokeFile(localResources_.at(resourceName));

}

void TorrentClient::sendMyState(sockaddr_in newPeer) {
    std::stringstream ss;
    localResourcesMutex.lock();
    char payload[MAX_SIZE_OF_PAYLOAD] = {};
    char sbuf[HEADER_SIZE + MAX_SIZE_OF_PAYLOAD] = {};
    for(const auto& [resourceName, resource] : localResources_){
        if(ss.str().size() + resourceName.size() > MAX_SIZE_OF_PAYLOAD){
            snprintf(payload, sizeof(payload), "%s", ss.str().c_str());
            memset(sbuf, 0 , sizeof(sbuf));
            snprintf(sbuf, sizeof(sbuf), "%d;%s", STATE_OF_NODE, payload);
            if (sendto(udpSocket, sbuf, strlen(sbuf) + 1, 0, (struct sockaddr *) &newPeer, sizeof newPeer) < 0) {
                errno_abort("send");
            }
            ss.clear();
        }
        ss << ";" << resource.resourceName;
    }

    localResourcesMutex.unlock();
    memset(sbuf, 0 , sizeof(sbuf));
    snprintf(sbuf, sizeof(sbuf), "%d;%s", STATE_OF_NODE, payload);

    if (sendto(udpSocket, sbuf, strlen(sbuf) + 1, 0, (struct sockaddr *) &newPeer, sizeof newPeer) < 0) {
        errno_abort("send");
    }

}

std::vector<ResourceInfo> TorrentClient::deserializeVectorOfResources(char *message)
{
    int pointer = 0;
    std::vector<ResourceInfo> resources;
    while (message[pointer] && pointer<=MAX_MESSAGE_SIZE)
    {
        resources.push_back(std::move(deserializeResource(message+pointer,true,&pointer)));
        pointer++;
    }

    // troche pozno jest kiedy to pisze ale czy powinienem sprawdzac dlugosc tego, co dostaje?
//    czy to nie jest zapewnione w zaden sposob wyzej? Jezeli tak to najmocniej przepraszam, mozna to wywalic
    if(pointer>MAX_MESSAGE_SIZE)
        throw std::runtime_error("message exceeded maximum lenght!");

    return resources;
}

ResourceInfo TorrentClient::deserializeResource(char *message, bool toVector,int *dataPointer) {
    std::string resourceName("");
    unsigned int sizeInBytes=0;
    std::size_t revokeHash=0;

    std::string builder("");

    unsigned short pointer=0;
    char currCharacter=message[pointer];

    while(currCharacter && currCharacter!=';'){
        resourceName+=currCharacter;
        currCharacter=message[++pointer];
    }

    //sprawdzanie czy nie ma konca pliku tam gdzie sie go nie spodziewamy
    if(!currCharacter)
        throw std::runtime_error("unexpected end of serialized data while reading resource name");

    currCharacter=message[++pointer];
    std::string sizeBuilder("");

    while(currCharacter && currCharacter!=';'){
        sizeBuilder+=currCharacter;
        currCharacter=message[++pointer];
    }
    try {
        sizeInBytes = std::stoi(sizeBuilder);
    }
    catch (std::exception exception){
        throw std::runtime_error("exceeded number value limit or invalid character read while reading resource size");
    }

    if(!currCharacter)
        throw std::runtime_error("unexpected end of serialized data while reading resource size");

    currCharacter=message[++pointer];
    std::string revokeHashBuilder("");


    if(toVector){
        //do wektora to czekamy na albo NULL albo na srednik (;)
        while(currCharacter && currCharacter!=';'){
            revokeHashBuilder+=currCharacter;
            currCharacter=message[++pointer];
        }
    }
    else
    {
        //nie do wektora to czekamy na NULL znak
        while(currCharacter){
            revokeHashBuilder+=currCharacter;
            currCharacter=message[++pointer];
        }
    }
    try {
        revokeHash = std::stoi(revokeHashBuilder);
    }
    catch (std::exception exception){
        throw std::runtime_error("exceeded number value limit or invalid character read while reading resource revoke hash");
    }
    if(toVector)
        *dataPointer+=pointer;

    if(pointer>MAX_MESSAGE_SIZE)
        throw std::runtime_error("message exceeded maximum lenght!");

    return ResourceInfo(resourceName,
                        sizeInBytes,
                        revokeHash);
}

DemandChunkMessage TorrentClient::deserializeChunkMessage(char *message) {
    //zakladam, ze tutaj struktura jest "name;index1;index2;...indexn;000..."
    std::string name("");
    std::vector<unsigned int> indices;

    unsigned short pointer=0;
    char currCharacter=message[pointer];

    while(currCharacter && currCharacter!=';'){
        name+=currCharacter;
        currCharacter=message[++pointer];
    }
    //sprawdzanie czy nie ma konca pliku tam gdzie sie go nie spodziewamy
    if(!currCharacter)
        throw std::runtime_error("unexpected end of serialized data while reading chunk message name");
    //przechodzimy przez srednik
    currCharacter=message[++pointer];

    std::string currentIndex("");

    while(currCharacter){
        if(isdigit(currCharacter)){
            currentIndex+=currCharacter;
        } else if(currCharacter==';'){
            //mamy nowy index - wrzucamy do wektora i resetujemy stringa zbierajacego cyfry
            try {
                indices.push_back(std::stoi(currentIndex));
            }
            catch(std::exception exception){
                throw std::runtime_error("Exceeded uint limit or invalid character while reading index");
            }
            currentIndex="";
        } else{
            //ani nie cyfra ani nie srednik
            throw std::runtime_error("Unexpected character while reading chunk message indices");
        }
        currCharacter=message[++pointer];
    }
    if(!std::empty(currentIndex)){
        try {
            indices.push_back(std::stoi(currentIndex));
        }
        catch(std::exception exception){
            throw std::runtime_error("Exceeded uint limit or invalid character while reading index");
        }
    }
    return DemandChunkMessage(name,
                              indices);
}

void errno_abort(const std::string &header){
    perror(header.c_str());
    exit(EXIT_FAILURE);
}


