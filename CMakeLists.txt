cmake_minimum_required(VERSION 3.25)
project(ServerProject)

set(CMAKE_CXX_STANDARD 11)

add_executable(ServerProject main.cpp src/Log.cpp include/util.h src/util.cpp include/singleton.h include/config.h src/config.cpp include/thread.h src/thread.cpp include/srvpro.h include/macro.h include/fiber.h src/fiber.cpp include/mutex.h src/mutex.cpp include/scheduler.h src/scheduler.cpp include/iomanager.h src/iomanager.cpp include/timer.h src/timer.cpp include/hook.h src/hook.cpp include/fd_manager.h src/fd_manager.cpp include/noncopyable.h include/address.h src/address.cpp include/endian.h)

include_directories(include)

target_link_libraries(ServerProject /usr/local/lib/libyaml-cpp.so)