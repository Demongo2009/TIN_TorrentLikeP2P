#include <iostream>
#include "../../include/utils/CliParser.h"


ClientCommand CliParser::parseCommand(std::vector<std::string> commandWords, std::string &filepath, std::string &resourceName){
    if(commandWords.empty()){
        return UNKNOWN;
    }
    std::string command = commandWords[0];
    if(command == "new") {
        return parseNew(commandWords, filepath, resourceName);
    } else if(command == "list"){
        return LIST_AVAILABLE_RESOURCES;
    } else if(command == "find"){
        return parseFind(commandWords, resourceName);
    } else if(command == "download"){
        return parseDownload(commandWords, filepath, resourceName);
    } else if(command == "revoke"){
        return parseRevokeResource(commandWords, resourceName);
    } else if(command == "q"){
        return EXIT;
    }else{
        return UNKNOWN;
    }
}


bool CliParser::checkResourceNameLength(const std::string & resourceName) {
    if( resourceName.size() > MAX_FILE_NAME_SIZE){
        std::cout << "File name is too long!\n";
        return false;
    }
    return true;
}

ClientCommand CliParser::parseNew(const std::vector<std::string>& commandWords, std::string &filePath, std::string &resourceName) {
    if(commandWords.size() < 3){
        std::cout << "Invalid 'new' command! Please insert 'new <filePath> <resourceName>'\n";
        return UNKNOWN;
    }
    filePath = commandWords[1];
    resourceName = commandWords[2];
    if(!checkResourceNameLength(resourceName)){
        return UNKNOWN;
    }
    return ADD_NEW_RESOURCE;
}

ClientCommand CliParser::parseFind(const std::vector<std::string> &commandWords, std::string &resourceName) {
    if(commandWords.size() < 2){
        std::cout << "Invalid 'find' command! Please insert 'find <resourceName>'\n";
        return UNKNOWN;
    }
    resourceName = commandWords[1];
    if(!checkResourceNameLength(resourceName)){
        return UNKNOWN;
    }
    return FIND_RESOURCE;
}


ClientCommand CliParser::parseDownload(const std::vector<std::string> &commandWords, std::string &filePath, std::string &resourceName) {
    if(commandWords.size() < 3){
        std::cout << "Invalid 'new' command! Please insert 'download <resourceName> <filePath>'\n";
        return UNKNOWN;
    }
    resourceName = commandWords[1];
    filePath = commandWords[2];
    if(!checkResourceNameLength(resourceName)){
        return UNKNOWN;
    }
    return DOWNLOAD_RESOURCE;

}

ClientCommand CliParser::parseRevokeResource(const std::vector<std::string> &commandWords, std::string &resourceName) {
    if(commandWords.size() < 2){
        std::cout << "Invalid 'revoke' command! Please insert 'revoke <resourceName>'\n";
        return UNKNOWN;
    }
    resourceName = commandWords[1];
    if(!checkResourceNameLength(resourceName)){
        return UNKNOWN;
    }
    return REVOKE_RESOURCE;
}