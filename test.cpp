//
// Created by igor on 6/1/21.
//

#include "include/structs/Message.h"
#include "include/structs/ResourceInfo.h"
#include "include/threads/TorrentClient.h"
#include <iostream>

int main()
{
    char* resourceVectorString =
                "testowa_wartosc_nr1;997;112;"
                "test_nr_2;1;2;"
                "testTrzeci;2;4;"
                "TESTCZWARTY__;100000;20000;"
                "t e s t p i a t y;30;100000;"
                "tescik szosty ;45;0;"
                "test             nr 7;43;43;";

    char* resourceString =
                "wartosc;100;300;";
    char* chunkMsgString =
                "nazwa;"
                "13;"
                "14;"
                "0;"
                "23;"
                "997;"
                "21235432;"
                "997;"
                "112;"
                "1;"
                "2137;"
                "3;";

    TorrentClient klient;

    ResourceInfo dummyResource;
    DemandChunkMessage message = klient.deserializeChunkMessage(chunkMsgString);
    ResourceInfo resource = klient.deserializeResource(resourceString);
    std::vector<ResourceInfo> resourceVector = klient.deserializeVectorOfResources(resourceVectorString);

    std::cout<<"tests passed!\n";



return 0;
}


