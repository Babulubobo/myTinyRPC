#include "myRPC/net/timer_event.h"
#include "myRPC/common/util.h"
#include "myRPC/common/log.h"

namespace myRPC {

TimerEvent::TimerEvent(int interval, bool is_repeated, std::function<void()> cb)
 : m_interval(interval), m_is_repeated(is_repeated), m_task(cb) {
    resetArriveTime(); // now time(ms) + interval time(ms)
    
}

TimerEvent::~TimerEvent(){
    
}

void TimerEvent::resetArriveTime() {
    m_arrive_time = getNowMs() + m_interval;
    DEBUGLOG("success create timer event, will excute at [%lld]", m_arrive_time);
}

}