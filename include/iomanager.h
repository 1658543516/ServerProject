//
// Created by mrpiao on 23-7-4.
//

#ifndef SERVERPROJECT_IOMANAGER_H
#define SERVERPROJECT_IOMANAGER_H

#include "scheduler.h"
#include "timer.h"
//#include <iostream>
//#include <vector>

namespace srvpro {

class IOManager : public Scheduler, public TimerManager {

public:
    typedef std::shared_ptr<IOManager> ptr;
    typedef RWMutex RWMutexType;
    
    enum Event {
    	NONE = 0x0,
    	READ = 0x1,
    	WRITE = 0x4,
    };
    
private:
    struct FdContext {
        typedef Mutex MutexType;
        struct EventContext {
            Scheduler* scheduler = nullptr;
            Fiber::ptr fiber;
            std::function<void()> cb;
        };
        
        EventContext& getContext(Event event);
        void resetContext(EventContext& ctx);
        void triggerEvent(Event event);
        
        EventContext read;
        EventContext write;
        int fd = 0;
        Event events = NONE;
        MutexType mutex;
    };

public:
    IOManager(size_t threads = 1, bool use_caller = true, const std::string& name = "");
    ~IOManager();
    
    int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
    bool delEvent(int fd, Event event);
    bool cancelEvent(int fd, Event event);
    
    bool cancelAll(int fd);
    
    static IOManager* GetThis();

protected:
    void tickle() override;
    bool stopping() override;
    bool stopping(uint64_t& timeout);
    void idle() override;
    void onTimerInsertedAtFront() override;
    
    void contextResize(size_t size);
private:
    int m_epfd = 0;
    int m_tickleFds[2];
    
    std::atomic<size_t> m_pendingEventCount = {0};
    RWMutexType m_mutex;
    std::vector<FdContext*> m_fdContexts;

};

}

#endif //SERVERPROJECT_IOMANAGER_H
