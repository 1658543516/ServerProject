cmake_minimum_required(VERSION 3.25)
project(ServerProject)

set(CMAKE_CXX_STANDARD 17)

add_executable(ServerProject main.cpp src/Log.cpp)

include_directories(include)
