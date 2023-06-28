//
// Created by mrpiao on 23-6-28.
//

#include "Log.h"
#include "scheduler.h"
#include "macro.h"
#include "util.h"

namespace srvpro {

    static srvpro::Logger::ptr g_logger = SRVPRO_LOG_NAME("system");
    
    static thread_local Scheduler* t_scheduler = nullptr;
    static thread_local Fiber* t_fiber = nullptr;
    
    Scheduler::Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "")
    	:m_name(name) {
    	SRVPRO_ASSERT1(threads > 0);
    	
    	if(use_caller) {
    	    srvpro::Fiber::GetThis();
    	    --threads;
    	    
    	    SRVPRO_ASSERT1(GetThis() == nullptr);
    	    t_scheduler = this;
    	    
    	    m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this)));
    	    srvpro::Thread::SetName(m_name);
    	    
    	    t_fiber = m_rootFiber.get();
    	    m_rootThread = srvpro::GetThreadID();
    	    m_threadIds.push_back(m_rootThread);
    	} else {
    	    m_rootThread = -1;
    	}
    	m_threadCount = threads;
    }
    
    Scheduler::~Scheduler() {
    	SRVPRO_ASSERT1(m_stopping);
    	if(GetThis() == this) {
    	    t_scheduler = nullptr;
    	}
    }
    	    
    Scheduler* Scheduler::GetThis() {
    	return t_scheduler;
    }
    
    Fiber* Scheduler::GetMainFiber() {
    	return t_fiber;
    } 
    	    
    void Scheduler::start() {
    	MutexType::Lock lock(m_mutex);
    	if(!m_stopping) {
    	    return;
    	}
    	m_stopping = false;
    	SRVPRO_ASSERT1(m_threads.empty());
    	
    	m_threads.resize(m_threadCount);
    	for(size_t i = 0; i < m_threadCount; ++i) {
    	    m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this), m_name + "_" + std::to_string(i)));
    	    m_threadIds.push_back(m_threads[i]->getId());
    	}
    }
    
    void Scheduler::stop() {
    	m_autoStop = true;
    	if(m_rootFiber && m_threadCount == 0 && (m_rootFiber->getState == Fiber::TERM || m_rootFiber->getState == Fiber::INIT)) {
    	    SRVPRO_LOG_INFO(g_logger) << this << " stopped";
    	    m_stopping = true;
    	    
    	    if(stopping()) {
    	    	return;
    	    }
    	}
    	
    	bool exit_on_this_fiber = false;
    	if(m_rootThread != -1) {
    	    SRVPRO_ASSERT1(GetThis() == this);
    	} else {
    	    SRVPRO_ASSERT1(GetThis() != this);
    	}
    	
    	m_stopping = true;
    	for(size_t i = 0; i < m_threadCount; ++i) {
    	    tickle();
    	}
    	
    	if(m_rootFiber) {
    	    tickle();
    	}
    }
    
    void Scheduler::setThis() {
    	t_scheduler = this;
    }
    
    void Scheduler::run() {
    	setThis();
    	if(srvpro::GetThreadID() != m_rootThread) {
    	    t_fiber = Fiber::GetThis().get();
    	}
    	
    	Fiber::ptr idle_fiber(new Fiber());
    }

}
