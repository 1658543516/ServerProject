//
// Created by mrpiao on 23-5-31.
//
//#include <csignal>
#include <syscall.h>
#include <csignal>
#include "util.h"

namespace srvpro {
    pid_t GetThreadID() {
        return syscall(SYS_gettid);
    }

    uint32_t GetFiberID() {
        return 0;
    }
}