//
// Created by bartlomiej on 09.06.2021.
//

#ifndef TIN_TORRENTLIKEP2P_SYNCHRONIZEDFILE_H
#define TIN_TORRENTLIKEP2P_SYNCHRONIZEDFILE_H
#include <mutex>
#include <fstream>


class SynchronizedFile{
public:
    SynchronizedFile(const std::string& filename){
        this->filename = filename;
    }

    SynchronizedFile(const SynchronizedFile& other){
        this->filename = other.getFilename();
    }

    void write(const char * data, unsigned int index){
        std::lock_guard<std::mutex> lock(writerMutex);
        std::ofstream ofs (filename, std::ios::in | std::ofstream::out | std::ofstream::binary);
        int c = CHUNK_SIZE;
        long offset = index * c;
        ofs.seekp(offset, std::ios::beg);
        ofs<<data;
        ofs.close();
    }

    std::string getFilename()const {
        return filename;
    }

    void reserveFile(unsigned long long fileSize){
        char buffer[] = {"1"};
        FILE * pFile = std::fopen (filename.c_str(), "wb");
        for(unsigned long long i = 0; i < fileSize; ++i) {
            std::fwrite(buffer, sizeof(char), 1, pFile);
        }
        std::fclose (pFile);
    }

private:
    std::string filename;
    std::mutex writerMutex;


};

#endif //TIN_TORRENTLIKEP2P_SYNCHRONIZEDFILE_H
