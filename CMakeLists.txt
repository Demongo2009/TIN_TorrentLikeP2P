cmake_minimum_required(VERSION 3.17)
project(TIN_TorrentLikeP2P)

set(CMAKE_CXX_STANDARD 17)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_CXX_FLAGS -pthread)

FILE(GLOB_RECURSE TIN_P2P_TORRENTS_SOURCES src/*/*.cpp src/*.cpp)
ADD_LIBRARY(torrents-objects OBJECT ${TIN_P2P_TORRENTS_SOURCES})
ADD_EXECUTABLE(tin-p2p-torrents $<TARGET_OBJECTS:torrents-objects> main.cpp)

ADD_EXECUTABLE(DESERIALIZE_TEST
        src/threads/TorrentClient.cpp test.cpp include/threads/TorrentClient.h)
TARGET_LINK_LIBRARIES(tin-p2p-torrents pthread)


