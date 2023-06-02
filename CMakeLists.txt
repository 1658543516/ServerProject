cmake_minimum_required(VERSION 3.25)
project(ServerProject)

set(CMAKE_CXX_STANDARD 11)

add_executable(ServerProject main.cpp src/Log.cpp include/util.h src/util.cpp include/singleton.h include/config.h)

include_directories(include)

target_link_libraries(ServerProject /usr/local/lib/libyaml-cpp.so)