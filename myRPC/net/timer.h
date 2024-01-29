#ifndef MYRPC_NET_TIMER_H
#define MYRPC_NET_TIMER_H

#include <map>
#include "myRPC/common/mutex.h"
#include "myRPC/net/fd_event.h"
#include "myRPC/net/timer_event.h"

namespace myRPC
{
    
class Timer : public FdEvent {
public:

    Timer();
    ~Timer();

    void addTimerEvent(TimerEvent::s_ptr event);

    void delTimerEvent(TimerEvent::s_ptr event);

    void onTimer(); // do the callback function in TimerEvent when the time is right

private:
    void resetArriveTime();

private:

    // multimap: allow same keys, and the elements are ordered
    std::multimap<int64_t, TimerEvent::s_ptr> m_pending_events;

    Mutex m_mutex; // high performance: Readers-Writer Lock?

};

} // namespace myRPC


#endif