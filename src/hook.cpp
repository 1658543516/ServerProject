//
// Created by mrpiao on 23-7-6.
//
#include "hook.h"
#include <dlfcn.h>

#include "config.h"
#include "Log.h"
#include "fiber.h"
#include "iomanager.h"
#include "fd_manager.h"
#include "macro.h"


namespace srvpro {
    srvpro::Logger::ptr g_logger = SRVPRO_LOG_NAME("system");
    static thread_local bool t_hook_enable = false;

#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep) \
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt)

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

    struct timer_info {
        int cancelled = 0;
    };

    template<typename OriginFun, typename ... Args>
    static ssize_t do_io(int fd, OriginFun fun, const char *hook_fun_name, uint32_t event, int timeout_so, Args &&... args) {
        if (!srvpro::t_hook_enable) {
            return fun(fd, std::forward<Args>(args)...);
        }

        srvpro::FdCtx::ptr ctx = srvpro::FdMgr::GetInstance()->get(fd);
        if (!ctx) {
            return fun(fd, std::forward<Args>(args)...);
        }

        if (ctx->isClose()) {
            errno = EBADF;
            return -1;
        }

        if (!ctx->isSocket() || ctx->getUserNonblock()) {
            return fun(fd, std::forward<Args>(args)...);
        }

        uint64_t to = ctx->getTimeout(timeout_so);
        std::shared_ptr<timer_info> tinfo(new timer_info);

        retry:
        ssize_t n = fun(fd, std::forward<Args>(args)...);
        while (n == -1 && errno == EINTR) {
            n = fun(fd, std::forward<Args>(args)...);
        }
        if (n == -1 && errno == EAGAIN) {
            srvpro::IOManager *iom = srvpro::IOManager::GetThis();
            srvpro::Timer::ptr timer;
            std::weak_ptr<timer_info> winfo(tinfo);

            if (to != (uint64_t) -1) {
                timer = iom->addConditionTimer(to, [winfo, fd, iom, event]() {
                    auto t = winfo.lock();
                    if (!t || t->cancelled) {
                        return;
                    }
                    t->cancelled = ETIMEDOUT;
                    iom->cancelEvent(fd, (srvpro::IOManager::Event) (event));
                }, winfo);
            }

            int rt = iom->addEvent(fd, (srvpro::IOManager::Event) (event));
            if (rt) {
                SRVPRO_LOG_ERROR(g_logger) << hook_fun_name << " addEvent("
                                           << fd << ", " << event << ")";
                if (timer) {
                    timer->cancel();
                }
                return -1;
            } else {
                srvpro::Fiber::YieldToHold();
                if (timer) {
                    timer->cancel();
                }
                if (tinfo->cancelled) {
                    errno = tinfo->cancelled;
                    return -1;
                }
                goto retry;
            }
        }

        return n;
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
    iom->addTimer(seconds * 1000, std::bind((void (srvpro::Scheduler::*)
            (srvpro::Fiber::ptr, int thread)) &srvpro::IOManager::schedule, iom, fiber, -1));
    srvpro::Fiber::YieldToHold();
    return 0;
}

int usleep(useconds_t usec) {
    if (!srvpro::t_hook_enable) {
        return usleep_f(usec);
    }
    srvpro::Fiber::ptr fiber = srvpro::Fiber::GetThis();
    srvpro::IOManager *iom = srvpro::IOManager::GetThis();
    iom->addTimer(usec / 1000, std::bind((void (srvpro::Scheduler::*)
            (srvpro::Fiber::ptr, int thread)) &srvpro::IOManager::schedule, iom, fiber, -1));
    srvpro::Fiber::YieldToHold();
    return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
    if(!srvpro::t_hook_enable) {
        return nanosleep_f(req, rem);
    }

    int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 /1000;
    srvpro::Fiber::ptr fiber = srvpro::Fiber::GetThis();
    srvpro::IOManager* iom = srvpro::IOManager::GetThis();
    iom->addTimer(timeout_ms, std::bind((void(srvpro::Scheduler::*)
                                                (srvpro::Fiber::ptr, int thread))&srvpro::IOManager::schedule
            ,iom, fiber, -1));
    srvpro::Fiber::YieldToHold();
    return 0;
}

int socket(int domain, int type, int protocol) {
    if(!srvpro::t_hook_enable) {
        return socket_f(domain, type, protocol);
    }
    int fd = socket_f(domain, type, protocol);
    if(fd == -1) {
        return fd;
    }
    srvpro::FdMgr::GetInstance()->get(fd, true);
    return fd;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    return connect_f(sockfd, addr, addrlen);
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
    int fd = do_io(s, accept_f, "accept", srvpro::IOManager::READ, SO_RCVTIMEO, addr, addrlen);
    if(fd >= 0) {
        srvpro::FdMgr::GetInstance()->get(fd, true);
    }
    return fd;
}

ssize_t read(int fd, void *buf, size_t count) {
    return do_io(fd, read_f, "read", srvpro::IOManager::READ, SO_RCVTIMEO, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, readv_f, "readv", srvpro::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return do_io(sockfd, recv_f, "recv", srvpro::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    return do_io(sockfd, recvfrom_f, "recvfrom", srvpro::IOManager::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    return do_io(sockfd, recvmsg_f, "recvmsg", srvpro::IOManager::READ, SO_RCVTIMEO, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return do_io(fd, write_f, "write", srvpro::IOManager::WRITE, SO_SNDTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, writev_f, "writev", srvpro::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags) {
    return do_io(s, send_f, "send", srvpro::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags);
}

ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
    return do_io(s, sendto_f, "sendto", srvpro::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
    return do_io(s, sendmsg_f, "sendmsg", srvpro::IOManager::WRITE, SO_SNDTIMEO, msg, flags);
}

int close(int fd) {
    if(!srvpro::t_hook_enable) {
        return close_f(fd);
    }

    srvpro::FdCtx::ptr ctx = srvpro::FdMgr::GetInstance()->get(fd);
    if(ctx) {
        auto iom = srvpro::IOManager::GetThis();
        if(iom) {
            iom->cancelAll(fd);
        }
        srvpro::FdMgr::GetInstance()->del(fd);
    }
    return close_f(fd);
}
}
