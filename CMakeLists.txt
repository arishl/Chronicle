cmake_minimum_required(VERSION 3.16)
project(Chronicle)

# C++ Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Library source files
set(CHRONICLE_SOURCES
    src/AsyncLogger/AsyncLogger.cpp
    src/AsyncLogger/LogLevel.cpp
    src/AsyncLogger/LogMessage.cpp
)

set(CHRONICLE_HEADERS
    include/AsyncLogger/AsyncLogger.hpp
    include/AsyncLogger/LogLevel.hpp
    include/AsyncLogger/LogMessage.hpp
    include/RingBuffer/RingBuffer.hpp
)

# Create static library
add_library(chronicle STATIC ${CHRONICLE_SOURCES})
add_library(Chronicle::chronicle ALIAS chronicle)

target_include_directories(chronicle
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(chronicle PUBLIC cxx_std_23)

# Platform-specific settings
if(UNIX AND NOT APPLE)
    target_link_libraries(chronicle PUBLIC pthread)
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(chronicle PRIVATE /W4)
else()
    target_compile_options(chronicle PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Installation
include(GNUInstallDirs)

install(TARGETS chronicle
    EXPORT ChronicleTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)

install(EXPORT ChronicleTargets
    FILE ChronicleTargets.cmake
    NAMESPACE Chronicle::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Chronicle
)

# Create config file
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ChronicleConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ChronicleConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Chronicle
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ChronicleConfigVersion.cmake
    VERSION 1.0.0
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ChronicleConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ChronicleConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Chronicle
)

# Demo executable and benchmarks (optional)
option(CHRONICLE_BUILD_EXAMPLES "Build example programs" ON)

if(CHRONICLE_BUILD_EXAMPLES)
    add_subdirectory(Examples)
endif()
