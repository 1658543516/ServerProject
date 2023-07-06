//
// Created by mrpiao on 23-7-6.
//
#include "hook.h"
#include <dlfcn.h>

#include "config.h"
#include "Log.h"
#include "fiber.h"
#include "iomanager.h"
#include "macro.h"


namespace srvpro {
    static thread_local bool t_hook_enable = false;

#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep)

    void hook_init() {
        static bool is_inited = false;
        if (is_inited) {
            return;
        }
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
        HOOK_FUN(XX);
#undef XX
    }

//static uint64_t s_connect_timeout = -1;
    struct _HookIniter {
        _HookIniter() {
            hook_init();
            //s_connect_timeout = g_tcp_connect_timeout->getValue();
//
            //g_tcp_connect_timeout->addListener([](const int& old_value, const int& new_value){
            //        SYLAR_LOG_INFO(g_logger) << "tcp connect timeout changed from "
            //                                << old_value << " to " << new_value;
            //       s_connect_timeout = new_value;
            // });
        }
    };

    static _HookIniter s_hook_initer;

    bool is_hook_enable() {
        return t_hook_enable;
    }

    void set_hook_enable(bool flag) {
        t_hook_enable = flag;
    }
}

extern "C" {
#define XX(name) name ## _fun name ## _f = nullptr;
HOOK_FUN(XX) ;
#undef XX

unsigned int sleep(unsigned int seconds) {
    if (!srvpro::t_hook_enable) {
        return sleep_f(seconds);
    }

    srvpro::Fiber::ptr fiber = srvpro::Fiber::GetThis();
    srvpro::IOManager *iom = srvpro::IOManager::GetThis();
    iom->addTimer(seconds * 1000, [iom, fiber](){
        iom->schedule(fiber);
    });
    srvpro::Fiber::YieldToHold();
    return 0;
}

int usleep(useconds_t usec) {
    if (!srvpro::t_hook_enable) {
        return usleep_f(usec);
    }
    srvpro::Fiber::ptr fiber = srvpro::Fiber::GetThis();
    srvpro::IOManager *iom = srvpro::IOManager::GetThis();
    iom->addTimer(usec / 1000, [iom, fiber](){
        iom->schedule(fiber);
    });
    srvpro::Fiber::YieldToHold();
    return 0;
}
}
