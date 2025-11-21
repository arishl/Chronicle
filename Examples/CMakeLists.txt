cmake_minimum_required(VERSION 4.0)
project(LFRBLogging)

set(CMAKE_CXX_STANDARD 23)

add_executable(LFRBLogging demo.cpp
        ../include/RingBuffer/RingBuffer.hpp
        ../include/AsyncLogger/AsyncLogger.hpp
        ../src/AsyncLogger/AsyncLogger.cpp
        ../benchmarks/AsyncBenchmarker.cpp
        ../benchmarks/AsyncBenchmarker.hpp
        ../src/AsyncLogger/LogLevel.cpp
        ../include/AsyncLogger/LogLevel.hpp
        ../src/AsyncLogger/LogMessage.cpp
        ../include/Allocator/Allocator.hpp
        ../src/Allocator/Allocator.cpp)
