//
// Created by mrpiao on 23-6-28.
//

#include "Log.h"
#include "scheduler.h"
#include "macro.h"
#include "util.h"
#include "hook.h"

namespace srvpro {

    static srvpro::Logger::ptr g_logger = SRVPRO_LOG_NAME("system");
    
    static thread_local Scheduler* t_scheduler = nullptr;
    static thread_local Fiber* t_fiber = nullptr;
    
    Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name)
    	:m_name(name) {
    	SRVPRO_ASSERT1(threads > 0);
    	
    	if(use_caller) {
    	    srvpro::Fiber::GetThis();
    	    --threads;
    	    
    	    SRVPRO_ASSERT1(GetThis() == nullptr);
    	    t_scheduler = this;
    	    
    	    m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
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
        lock.unlock();

        /*if(m_rootFiber) {
            m_rootFiber->call();
            SRVPRO_LOG_INFO(g_logger) << "call out";
        }*/
    }
    
    void Scheduler::stop() {
    	m_autoStop = true;
    	if(m_rootFiber && m_threadCount == 0 && (m_rootFiber->getState() == Fiber::TERM || m_rootFiber->getState() == Fiber::INIT)) {
    	    SRVPRO_LOG_INFO(g_logger) << this << " stopped";
    	    m_stopping = true;
    	    
    	    if(stopping()) {
    	    	return;
    	    }
    	}
    	
    	//bool exit_on_this_fiber = false;
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

        if(m_rootFiber) {
            /*while(!stopping()) {
                if (m_rootFiber->getState() == Fiber::TERM || m_rootFiber->getState() == Fiber::EXCEPT) {
                    m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
                    SRVPRO_LOG_INFO(g_logger) << " root fiber is term, reset";
                    t_fiber = m_rootFiber.get();
                }
                m_rootFiber->call();
            }*/
            if (!stopping()) {
                m_rootFiber->call();
            }
        }

        std::vector<Thread::ptr> thrs;
        {
            MutexType::Lock lock(m_mutex);
            thrs.swap(m_threads);
        }

        for (auto& i : thrs) {
            i->join();
        }

        /*if(stopping()) {
            return;
        }*/
    }
    
    void Scheduler::setThis() {
    	t_scheduler = this;
    }
    
    void Scheduler::run() {
        SRVPRO_LOG_INFO(g_logger) << "run";
        //return;
        set_hook_enable(true);
    	setThis();
    	if(srvpro::GetThreadID() != m_rootThread) {
    	    t_fiber = Fiber::GetThis().get();
    	}
    	
    	Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
    	Fiber::ptr cb_fiber;
    	
    	FiberAndThread ft;
    	while(true) {
    	    ft.reset();
    	    bool tickle_me = false;
            bool is_active = false;

    	    {
    	    	MutexType::Lock lock(m_mutex);
    	    	auto it = m_fibers.begin();
    	    	while(it != m_fibers.end()) {
    	    	    if(it->thread != -1 && it->thread != srvpro::GetThreadID()) {
    	    	    	++it;
    	    	    	tickle_me = true;
    	    	    	continue;
    	    	    }
    	    	    
    	    	    SRVPRO_ASSERT1(it->fiber || it->cb);
    	    	    
    	    	    if(it->fiber && it->fiber->getState() == Fiber::EXEC) {
    	    	    	++it;
    	    	    	continue;
    	    	    }
    	    	    
    	    	    ft = *it;
    	    	    m_fibers.erase(it++);
                    ++m_activeThreadCount;
                    is_active = true;
                    break;
    	    	}
    	    }
    	    
    	    if(tickle_me) {
    	        tickle();
    	    }
    	    
    	    if(ft.fiber && (ft.fiber->getState() != Fiber::TERM && ft.fiber->getState() != Fiber::EXCEPT)) {
    	    	//++m_activeThreadCount;
    	    	ft.fiber->swapIn();
    	    	--m_activeThreadCount;
    	    	
    	    	if(ft.fiber->getState() == Fiber::READY) {
    	    	    schedule(ft.fiber);
    	    	} else if(ft.fiber->getState() != Fiber::TERM && ft.fiber->getState() != Fiber::EXCEPT) {
    	    	    ft.fiber->m_state = Fiber::HOLD;
    	    	}
    	    	ft.reset();
    	    } else if(ft.cb) {
    	    	if(cb_fiber) {
    	    	    cb_fiber->reset(ft.cb);
    	    	} else {
    	    	    cb_fiber.reset(new Fiber(ft.cb));
    	    	    //ft.cb = nullptr;
    	    	}
    	    	ft.reset();
    	    	
    	    	//++m_activeThreadCount;
    	    	cb_fiber->swapIn();
    	    	--m_activeThreadCount;
    	    	
    	    	if(cb_fiber->getState() == Fiber::READY) {
    	    	    schedule(cb_fiber);
    	    	    cb_fiber.reset();
    	    	} else if(cb_fiber->getState() == Fiber::EXCEPT || cb_fiber->getState() == Fiber::TERM) {
    	    	    cb_fiber->reset(nullptr);
    	    	} else {//if(cb_fiber->getState() != Fiber::TERM) {
    	    	    cb_fiber->m_state = Fiber::HOLD;
    	    	    cb_fiber.reset();
    	    	}
    	    	
    	    } else {
                if(is_active) {
                    --m_activeThreadCount;
                    continue;
                }
    	    	if(idle_fiber->getState() == Fiber::TERM) {
    	    	    SRVPRO_LOG_INFO(g_logger) << "idle fiber term";
                    //idle_fiber.reset();
                    //tickle();
    	    	    break;
                    //continue;
    	    	}
    	    	
    	    	++m_idleThreadCount;
    	    	idle_fiber->swapIn();
    	    	--m_idleThreadCount;
    	    	if(idle_fiber->getState() != Fiber::TERM && idle_fiber->getState() != Fiber::EXCEPT) {
    	    	    idle_fiber->m_state = Fiber::HOLD;
    	    	}
    	    	
    	    }
    	    
    	}
    }
    
    void Scheduler::tickle() {
        SRVPRO_LOG_INFO(g_logger) << "tickle";
    }
    
    bool Scheduler::stopping() {
        MutexType::Lock lock(m_mutex);
        return m_autoStop && m_stopping && m_fibers.empty() && m_activeThreadCount == 0;
    }
    
    void Scheduler::idle() {
        SRVPRO_LOG_INFO(g_logger) << "idle";
        while(!stopping()) {
            srvpro::Fiber::YieldToHold();
    	}
    }


}
