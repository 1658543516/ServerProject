//
// Created by mrpiao on 23-5-31.
//

#ifndef SERVERPROJECT_UTIL_H
#define SERVERPROJECT_UTIL_H
#include <pthread.h>
#include <syscall.h>
#include <iostream>

namespace srvpro {
    pid_t GetThreadID();
    uint32_t GetFiberID();
}

#endif //SERVERPROJECT_UTIL_H
