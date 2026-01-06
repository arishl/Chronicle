cmake_minimum_required(VERSION 3.16)
project(LFRBLogging LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Simple libraries
add_library(allocator STATIC ../src/Allocator/Allocator.cpp)
target_include_directories(allocator PUBLIC ../include)

add_library(asynclogger STATIC
        ../src/AsyncLogger/AsyncLogger.cpp
        ../src/AsyncLogger/LogLevel.cpp
        ../src/AsyncLogger/LogMessage.cpp
)
target_include_directories(asynclogger PUBLIC ../include)
target_link_libraries(asynclogger PUBLIC allocator)

# Example
add_executable(LFRBLogging demo.cpp ../benchmarks/AsyncBenchmarker.cpp)
target_include_directories(LFRBLogging PRIVATE ../include)
target_link_libraries(LFRBLogging PRIVATE asynclogger allocator)

