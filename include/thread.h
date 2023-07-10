//
// Created by mrpiao on 23-6-19.
//

#ifndef SERVERPROJECT_THREAD_H
#define SERVERPROJECT_THREAD_H
#include <iostream>
#include <memory>
#include <functional>
#include <semaphore.h>
#include <atomic>
#include "mutex.h"
//#include "noncopyable.h"

namespace srvpro {

class Thread : Noncopyable {

public:
    typedef std::shared_ptr<Thread> ptr;
    Thread(std::function<void()> cb, const std::string& name);
    ~Thread();
    
    pid_t getId() const {return m_id;}
    const std::string& getName() const {return m_name;}
    
    void join();
    
    static Thread* GetThis();
    static const std::string& GetName();
    static void SetName(const std::string& name);

private:
    Thread(const Thread&) = delete;
    Thread(const Thread&&) = delete;
    Thread& operator=(const Thread&) = delete;
    static void* run(void* arg);

private:
    pid_t m_id = -1;
    pthread_t m_thread = 0;
    std::function<void()> m_cb;
    std::string m_name;
    
    Semaphore m_semaphore;
};

}

#endif //SERVERPROJECT_THREAD_H
