//
// Created by mrpiao on 23-6-19.
//
#include "thread.h"
#include "Log.h"
#include "util.h"

namespace srvpro {

static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOW";

static srvpro::Logger::ptr g_logger = SRVPRO_LOG_NAME("system");

Thread* Thread::GetThis() {
    return t_thread;
}

const std::string& Thread::GetName() {
    if(t_thread) {
        return t_thread->m_name;
    }
    return t_thread_name;
}

void Thread::SetName(const std::string& name) {
    if(t_thread) {
    	t_thread->m_name = name;
    }
    t_thread_name = name;
}

Thread::Thread(std::function<void()> cb, const std::string& name):m_cb(cb), m_name(name) {
    if(name.empty()) {
    	m_name = "UNKNOWN";
    }
    //t_thread_name = m_name;
    int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
    if(rt) {
    	SRVPRO_LOG_ERROR(g_logger) << "pthread_create thread fail, rt=" << rt << " name=" << name;
    	throw std::logic_error("pthread_create error");
    }
    m_semaphore.wait();
}

Thread::~Thread() {
    if(m_thread) {
    	pthread_detach(m_thread);
    }
}
    
void Thread::join() {
    if(m_thread) {
    	int rt = pthread_join(m_thread, nullptr);
    	if(rt) {
    	    SRVPRO_LOG_ERROR(g_logger) << "pthread_join thread fail, rt=" << rt << " name=" << m_name;
    	    throw std::logic_error("pthread_join error");
    	}
    	m_thread = 0;
    }
}
   
void* Thread::run(void* arg) {
    Thread* thread = (Thread*)arg;
    t_thread = thread;
    thread->m_id = srvpro::GetThreadID();
    pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());
    
    std::function<void()> cb;
    cb.swap(thread->m_cb);
    
    thread->m_semaphore.notify();
    
    cb();
    
    return 0;
}

}
