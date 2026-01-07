cmake_minimum_required(VERSION 3.16)
project(Chronicle LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Examples/demos for Chronicle logging library

add_executable(ChronicleDemo
        demo.cpp
        ../benchmarks/AsyncBenchmarker.cpp
)

target_link_libraries(ChronicleDemo PRIVATE Chronicle::chronicle)

install(TARGETS ChronicleDemo DESTINATION ${CMAKE_INSTALL_BINDIR})

