cmake_minimum_required(VERSION 4.0)
project(LFRBLogging)

set(CMAKE_CXX_STANDARD 23)

add_executable(LFRBLogging demo.cpp
        ../src/RingBuffer/RingBuffer.tpp
        ../include/RingBuffer/RingBuffer.h
        ../include/AsyncLogger/AsyncLogger.h
        ../src/AsyncLogger/AsyncLogger.cpp
        ../benchmarks/AsyncBenchmarker.cpp
        ../benchmarks/AsyncBenchmarker.h
        ../src/AsyncLogger/LogLevel.cpp
        ../include/AsyncLogger/LogLevel.h
        ../src/AsyncLogger/LogMessage.cpp)
