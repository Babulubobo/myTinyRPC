#ifndef MYRPC_NET_TIMEEVENT_H
#define MYRPC_NET_TIMEEVENT_H

#include <functional>
#include <memory>

namespace myRPC
{
    
class TimerEvent
{
private:
    int64_t m_arrive_time; // ms
    int64_t m_interval;    // ms
    bool m_is_repeated {false};
    bool m_is_canceled {false};

    std::function<void()> m_task;

public:
    typedef std::shared_ptr<TimerEvent> s_ptr; //another name of "std::shared_ptr<TimerEvent> s_ptr"  SHORTER!!!

    TimerEvent(int interval, bool is_repeated, std::function<void()> cb);
    ~TimerEvent();

    int64_t getArriveTime() {
        return m_arrive_time;
    }

    bool isCanceled() {
        return m_is_canceled;
    }

    bool isRepeated() {
        return m_is_repeated;
    }

    void setCanceled(bool value) {
        m_is_canceled = value;
    }

    std::function<void()> getCallBack() {
        return m_task;
    }

    void resetArriveTime();
};



} // namespace myRPC


#endif