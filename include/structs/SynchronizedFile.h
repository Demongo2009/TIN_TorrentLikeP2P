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

    std::string getFilename()const {
        return filename;
    }

    unsigned long long getSize()const {
        return size;
    }

    void reserveFile(unsigned long long fileSize);
    void write(const char * data, unsigned int index);

private:
    std::string filename;
    std::mutex writerMutex;
    unsigned long long size;
};

#endif //TIN_TORRENTLIKEP2P_SYNCHRONIZEDFILE_H
