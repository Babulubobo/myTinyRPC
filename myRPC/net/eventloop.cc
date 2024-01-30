#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <myRPC/net/eventloop.h>
#include <myRPC/common/log.h>
#include <myRPC/common/util.h>



#define ADD_TO_EPOLL() \
        auto it = m_listen_fds.find(event->getFd()); \
        int op = EPOLL_CTL_ADD; \
        if(it != m_listen_fds.end()) { \
            op = EPOLL_CTL_MOD; \
        } \
        epoll_event tmp = event->getEpollEvent(); \
        INFOLOG("epoll_event.events = %d", (int)tmp.events); \
        int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp); \
        if(rt == -1) { \
            ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s", errno, strerror(errno)); \
        } \
        DEBUGLOG("add event successfully, fd[%d]", event->getFd());\


// auto it = m_listen_fds.find(event->getFd()); // if the event has been added before
// int op = EPOLL_CTL_ADD; // default is add
// if(it != m_listen_fds.end()) {
//     op = EPOLL_CTL_MOD; // if exist, modify(EPOLL_CTL_MOD) instead of add(EPOLL_CTL_ADD)
// }
// epoll_event tmp = event->getEpollEvent();
// int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp);
// if(rt == -1) {
//     ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s", errno, strerror(errno));
// }

#define DELETE_TO_EPOLL() \
        auto it = m_listen_fds.find(event->getFd()); \
        if(it == m_listen_fds.end()) { \
            return;\
        }\
        int op = EPOLL_CTL_DEL;\
        epoll_event tmp = event->getEpollEvent();\
        int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp);\
        if(rt == -1) {\
            ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s", errno, strerror(errno));\
        }\
        DEBUGLOG("del event successfully, fd[%d]", event->getFd());\

namespace myRPC {

// if there is an Eventloop in this thread (1 thread 1 eventloop)
static thread_local Eventloop* t_current_eventloop = NULL;

static int g_epoll_max_timeout = 10000;//max 10 seconds
static int g_epoll_max_events = 10;    //max 10 events

Eventloop::Eventloop(){
    if(t_current_eventloop != NULL) {
        ERRORLOG("failed to create event loop, this thread has created event loop");
        exit(0);
    }
    m_thread_id = getThreadId();
    m_epoll_fd = epoll_create(10); // On error, -1 is returned

    if(m_epoll_fd == -1) {
        ERRORLOG("failed to create event loop, epoll_create error, error info[%d]", errno);
        exit(0);
    }

    initWakeUpFdEvent();
    initTimer();

    INFOLOG("success create event loop in thread %d", m_thread_id);

    t_current_eventloop = this;
}

Eventloop::~Eventloop(){
    close(m_epoll_fd);
    if(m_wakeup_fd_event) {
        delete m_wakeup_fd_event;
        m_wakeup_fd_event = nullptr;
    }

    if(m_timer) {
        delete m_timer;
        m_timer = nullptr;
    }
}


void Eventloop::initTimer() {
    m_timer = new Timer();
    addEpollEvent(m_timer); // ?
}


void Eventloop::addTimerEvent(TimerEvent::s_ptr event) {
    m_timer->addTimerEvent(event);
}


void Eventloop::initWakeUpFdEvent() {
    m_wakeup_fd = eventfd(0, EFD_NONBLOCK);
    if(m_wakeup_fd == -1) {
        ERRORLOG("failed to create event loop, eventfd error, error info[%d]", errno);
        exit(0);
    }
    INFOLOG("wakeup fd = %d", m_wakeup_fd);

    m_wakeup_fd_event = new WakeUpFdEvent(m_wakeup_fd);

    m_wakeup_fd_event->listen(FdEvent::IN_EVENT, [this]() {
        char buf[8];
        // non-block read 
        while(read(m_wakeup_fd, buf, 8) != -1 && errno != EAGAIN){ //when read==-1 or errno == EAGAIN means all have been read

        }
        DEBUGLOG("read full bytes from wakeup fd[%d]", m_wakeup_fd);
    });

    addEpollEvent(m_wakeup_fd_event);
}

void Eventloop::loop(){
    while(!m_stop_flag) {
        ScopeMutex<Mutex> eventLock(m_mutex);
        std::queue<std::function<void()>> tmp_tasks;
        m_pending_tasks.swap(tmp_tasks);
        eventLock.unlock();

        while(!tmp_tasks.empty()) {
            std::function<void()> cb = tmp_tasks.front();
            tmp_tasks.pop();
            if(cb) {
                cb();
            }
        }

        int timeout = g_epoll_max_timeout;
        epoll_event result_events[g_epoll_max_events];

        // DEBUGLOG("now epoll_wait begin");
        int rt = epoll_wait(m_epoll_fd, result_events, g_epoll_max_events, timeout);
        DEBUGLOG("now epoll_wait end, rt = %d", rt);

        if(rt < 0) {
            // epoll_wait error
            ERRORLOG("epoll_wait error, errno=", errno);
        }
        else {
            for(int i = 0; i < rt; i ++){
                epoll_event trigger_event = result_events[i];
                FdEvent* fd_event = static_cast<FdEvent*>(trigger_event.data.ptr);
                if(fd_event == nullptr) {
                    continue;
                }
                if(trigger_event.events & EPOLLIN) {
                    DEBUGLOG("fd %d trigger EPOLLIN event", fd_event->getFd());
                    addTask(fd_event->handler(FdEvent::IN_EVENT));
                }
                if(trigger_event.events & EPOLLOUT) {
                    DEBUGLOG("fd %d trigger EPOLLOUT event", fd_event->getFd());
                    addTask(fd_event->handler(FdEvent::OUT_EVENT));
                }
            }
        }
    }
}

void Eventloop::wakeup(){
    INFOLOG("wake up");
    m_wakeup_fd_event->wakeup();
}

void Eventloop::stop(){
    m_stop_flag = true;
}

void Eventloop::dealWakeUp() {

}

void Eventloop::addEpollEvent(FdEvent* event){
    if(isInLoopThread()) {
        ADD_TO_EPOLL();
    }
    else {
        auto cb = [this, event]() {
            ADD_TO_EPOLL();
        };
        addTask(cb, true);
    }
}

void Eventloop::delEpollEvent(FdEvent* event){
    if(isInLoopThread()) {
        DELETE_TO_EPOLL();
    }
    else {
        auto cb = [this, event]() {
            DELETE_TO_EPOLL();
        };
        addTask(cb, true);
    }
}

void Eventloop::addTask(std::function<void()> cb, bool is_wake_up /*= false*/){
    ScopeMutex<Mutex> lock(m_mutex);
    m_pending_tasks.push(cb);
    lock.unlock();
    if(is_wake_up) {
        wakeup();
    }
}

bool Eventloop::isInLoopThread() {
    return getThreadId() == m_thread_id;
}

}
