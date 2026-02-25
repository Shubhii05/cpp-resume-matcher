cmake_minimum_required(VERSION 3.16)

project(ResumeMatcher)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell Crow we are using standalone Asio
add_definitions(-DASIO_STANDALONE)

# Include header directories
include_directories(include)

# Collect all source files
file(GLOB SOURCES "src/*.cpp")
include_directories(include)
add_executable(server ${SOURCES})