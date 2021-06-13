#include "../../include/structs/SynchronizedFile.h"
#include "../../include/structs/Message.h"

void SynchronizedFile::write(const char * data, unsigned int index){
    std::lock_guard<std::mutex> lock(writerMutex);
    std::ofstream ofs (filename, std::ios::in | std::ofstream::out);
    int c = CHUNK_SIZE;
    long offset = index * c;
    ofs.seekp(offset, std::ios::beg);
    ofs<<data;
    ofs.close();
}


void SynchronizedFile::reserveFile(unsigned long long fileSize){
    this->size = fileSize;
    char buffer[] = {"1"};
    FILE * pFile = std::fopen (filename.c_str(), "wb");
    for(unsigned long long i = 0; i < fileSize; ++i) {
        std::fwrite(buffer, sizeof(char), 1, pFile);
    }
    std::fclose (pFile);
}