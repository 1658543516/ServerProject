//
// Created by mrpiao on 23-7-5.
//

#ifndef SERVERPROJECT_TIMER_H
#define SERVERPROJECT_TIMER_H

#include <memory>
#include "thread.h"

namespace srvpro {

class TimeManager;
class Timer : public std::enable_shared_from_this<Timer> {
friend class TimeManager;
public:
    typedef std::shared_ptr<Timer> ptr;
private:
    Timer(uint64_t ms, std::function<void()> cb, bool recurring, TimeManager* manager);
    Timer(uint64_t next);
    
    bool cancel();
    bool refresh();
    bool reset(uint64_t ms, bool from_now);
private:
    bool m_recurring = false;
    uint64_t m_ms = 0;
    uint64_t m_next = 0;
    std::function<void()> m_cb;
    TimeManager* m_manager = nullptr;
    
private:
    struct Comparator {
        bool operator() (const Timer::ptr& lhs, const Timer::ptr& rhs) const;
    };

};

class TimerManager {
friend class Timer;
public:
    typedef RWMutex RWMutexType;
    
    TimerManager();
    virtual ~TimerManager();
    
    Timer::ptr addTimer(uint64_t ms, std::function<void()> cb, bool recurring);
    Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool recurring);
    uint64_t getNextTimer();
    void listExpiredCb(std::vector<std::function<void()> >& cbs);
protected:
    virtual void onTimerInsertedAtFront() = 0;
    void addTimer(Timer::ptr val, RWMutexType::WriteLock& lock);
private:
    bool detectClockRollover(uint64_t now_ms);
private:
    RWMutexType m_mutex;
    std::set<Timer::ptr, Timer::Comparator> m_timers;
    bool m_tickled = false;
    uint64_t m_previouseTime = 0;
};

}

#endif //SERVERPROJECT_TIMER_H
