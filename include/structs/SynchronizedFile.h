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
        this->size = other.getSize();
    }

    std::string getFilename()const {
        return filename;
    }

    unsigned long long getSize()const {
        return size;
    }

    void reserveFile(unsigned long long fileSize);
    void write(const uint8_t * data, unsigned int index, unsigned int countBytesToWrite);

private:
    std::string filename;
    std::mutex writerMutex;
    unsigned long long size;
};

#endif //TIN_TORRENTLIKEP2P_SYNCHRONIZEDFILE_H
