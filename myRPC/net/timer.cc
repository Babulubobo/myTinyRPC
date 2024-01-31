#include <sys/timerfd.h>
#include <string.h>
#include "myRPC/net/timer.h"
#include "myRPC/common/log.h"
#include "myRPC/common/util.h"

namespace myRPC {

Timer::Timer() : FdEvent() {

    // CLOCK_MONOTONIC: isn't affected by the adjustment of system time
    // TFD_NONBLOCK: non-blocking mode;
    // TFD_CLOEXEC: close while exec function to avoid being inherited by unwanted child process;
    m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    DEBUGLOG("Timer fd=%d", m_fd);

    // std::bind(&Timer::onTimer, this): bind the function in class, equals to return  this->onTimer();
    // and "this->onTimer()" be used as the callback func trans into "listen"
    // put the fd "in event" to the eventloop
    listen(FdEvent::IN_EVENT, std::bind(&Timer::onTimer, this));

}



Timer::~Timer() {

}


void Timer::onTimer() {

    // Process the data in buffer to prevent trigger the "read event" second time
    char buf[8];
    while(1) {
        if((read(m_fd, buf, 8) == -1) && errno == EAGAIN) {
            break;
        }
    }

    // do the timer event task
    int64_t now = getNowMs();

    std::vector<TimerEvent::s_ptr> tmps;
    std::vector<std::pair<int64_t, std::function<void()>>> tasks;

    ScopeMutex<Mutex> lockOnTimer(m_mutex);
    auto it = m_pending_events.begin();

    for(; it != m_pending_events.end(); it ++) {
        if((*it).first <= now) {
            if(!(*it).second->isCanceled()) {
                tmps.push_back((*it).second);
                tasks.push_back(std::make_pair((*it).second->getArriveTime(), (*it).second->getCallBack()));
            }
        }
        else {
            // not arrive time
            break;
        }
    }

    m_pending_events.erase(m_pending_events.begin(), it); // erase element from begin to it(not include it)
    lockOnTimer.unlock();

    // put the repeated event to the m_pending_events again
    // to let the m_pending_events re ordered;
    for(auto i = tmps.begin(); i != tmps.end(); i ++) {
        if((*i)->isRepeated()) {
            (*i)->resetArriveTime();
            addTimerEvent(*i);
        }
    }

    resetArriveTime(); // not the timer_event::reresetArriveTime, but the Timer::resetArriveTime

    for(auto i : tasks) {
        if(i.second) {
            i.second();
        }
    }

}

void Timer::resetArriveTime() {
    ScopeMutex<Mutex> timerResetLock(m_mutex);
    auto tmp = m_pending_events;
    timerResetLock.unlock();

    if(tmp.size() == 0) {
        return;
    }
    
    int64_t now = getNowMs();
    auto it = tmp.begin();
    int64_t inteval = 0;
    if((*it).second->getArriveTime() > now) {
        inteval = (*it).second->getArriveTime() - now;
    }
    else {
        // the first event in multiset is expired, set 100ms to do the event
        inteval = 100;
    }
    
    timespec ts;
    memset(&ts, 0, sizeof(ts));
    ts.tv_sec = inteval / 1000;
    ts.tv_nsec = (inteval % 1000) * 1000000;

    itimerspec value;
    memset(&value, 0, sizeof(value));
    value.it_value = ts;

    int rt = timerfd_settime(m_fd, 0, &value, NULL);
    if(rt != 0) {
        ERRORLOG("timerfd settime error, errno = %d, error = %s", errno, strerror(errno));
    }
    DEBUGLOG("timer reset to %lld", now + inteval);
}


void Timer::addTimerEvent(TimerEvent::s_ptr event) {

    bool is_reset_timerfd = false;
    ScopeMutex<Mutex> timerAddLock(m_mutex);

    if(m_pending_events.empty()) {
        is_reset_timerfd = true;
    }
    else {
        auto it = m_pending_events.begin();
        if((*it).second->getArriveTime() > event->getArriveTime()) {
            //the new event arrive time is earlier than the first event, the arrive time need to update
            is_reset_timerfd = true;
        }
    }

    m_pending_events.emplace(event->getArriveTime(), event);
    timerAddLock.unlock();

    if(is_reset_timerfd == true) {
    resetArriveTime();
    }
}

void Timer::delTimerEvent(TimerEvent::s_ptr event) {
    event->setCanceled(true);

    ScopeMutex<Mutex> timerDelLock(m_mutex);

    // multiset may have many event that has the same key(m_arrive_time)
    auto begin = m_pending_events.lower_bound(event->getArriveTime()); // lower_bound: return first element that >= bound
    auto end = m_pending_events.upper_bound(event->getArriveTime());   // upper_bound: return first element that > bound

    auto it = begin;
    for(; it != end; it ++){
        if(it->second == event) break;

    }

    if(it != end) {
        m_pending_events.erase(it);
        DEBUGLOG("success delete Timerevent at arrive time %lld", event->getArriveTime());
    }
    timerDelLock.unlock();
}

}

