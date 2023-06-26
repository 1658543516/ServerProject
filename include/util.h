//
// Created by mrpiao on 23-5-31.
//

#ifndef SERVERPROJECT_UTIL_H
#define SERVERPROJECT_UTIL_H
#include <pthread.h>
#include <syscall.h>
#include <iostream>
#include <vector>

namespace srvpro {
    pid_t GetThreadID();
    uint32_t GetFiberID();
    
    void Backtrace(std::vector<std::string>& bt, int size, int skip = 1);
    std::string BacktraceToString(int size, int skip = 2, const std::string& prefix = "");
}

#endif //SERVERPROJECT_UTIL_H
