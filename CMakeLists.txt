cmake_minimum_required(VERSION 4.0)
project(LFRBLogging)

set(CMAKE_CXX_STANDARD 23)

add_executable(LFRBLogging src/main.cpp
        src/RingBuffer.tpp
        include/RingBuffer.h)
