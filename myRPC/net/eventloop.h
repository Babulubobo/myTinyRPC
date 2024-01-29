#ifndef MYRPC_NET_EVENTLOOP_H
#define MYRPC_NET_EVENTLOOP_H

#include <functional>
#include <queue>
#include <set>
#include <pthread.h>

#include "myRPC/common/mutex.h"
#include "myRPC/net/fd_event.h"
#include "myRPC/net/wakeup_fd_event.h"
#include "myRPC/net/timer.h"

namespace myRPC {
class Eventloop {
public:
    Eventloop();
    ~Eventloop();

    void loop();

    void wakeup();

    void stop();

    void addEpollEvent(FdEvent* event);

    void delEpollEvent(FdEvent* event);

    bool isInLoopThread(); // other threads' event can't add directly

    void addTask(std::function<void()> cb, bool is_wake_up = false);

    void addTimerEvent(TimerEvent::s_ptr event);

private:
    void dealWakeUp();

    void initWakeUpFdEvent();

    void initTimer();

private:
    pid_t m_thread_id {0};

    WakeUpFdEvent* m_wakeup_fd_event {nullptr};

    bool m_stop_flag {false}; // the flag of whether the eventloop is stop

    int m_epoll_fd {0};

    int m_wakeup_fd {0}; // to wake up the epoll_wait

    std::set<int> m_listen_fds;

    std::queue<std::function<void()>> m_pending_tasks; // task list need to do

    Mutex m_mutex; // mutex used in the eventloop 

    Timer* m_timer {nullptr};

};
}

#endif