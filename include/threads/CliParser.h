#ifndef TIN_TORRENTLIKEP2P_CLIPARSER_H
#define TIN_TORRENTLIKEP2P_CLIPARSER_H

#include <string>
#include <vector>

const int MAX_FILE_NAME_SIZE = 256;

enum ClientCommand {
    ADD_NEW_RESOURCE,
    LIST_AVAILABLE_RESOURCES,
    FIND_RESOURCE,
    DOWNLOAD_RESOURCE,
    REVOKE_RESOURCE,
    EXIT,
    UNKNOWN
};

class CliParser {
public:
    static ClientCommand parseCommand(std::vector<std::string> , std::string &, std::string &);

private:
    static bool checkResourceNameLength(const std::string&) ;
    static ClientCommand parseNew(const std::vector<std::string>& , std::string &, std::string &) ;
    static ClientCommand parseFind(const std::vector<std::string>& , std::string &) ;
    static ClientCommand parseDownload(const std::vector<std::string>& , std::string &, std::string &) ;
    static ClientCommand parseRevokeResource(const std::vector<std::string>& , std::string &) ;
};


#endif //TIN_TORRENTLIKEP2P_CLIPARSER_H
