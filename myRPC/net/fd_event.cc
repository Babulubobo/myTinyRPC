#include <string.h>
#include <fcntl.h>
#include "myRPC/net/fd_event.h"
#include "myRPC/common/log.h"

namespace myRPC {

FdEvent::FdEvent(int fd) : m_fd(fd) {
    memset(&m_listen_events, 0, sizeof(m_listen_events));
}

FdEvent::FdEvent() {
    memset(&m_listen_events, 0, sizeof(m_listen_events));
}


FdEvent::~FdEvent() {

}

std::function<void()> FdEvent::handler(TriggerEvent event_type) {
    if(event_type == TriggerEvent::IN_EVENT) {
        return m_read_callback;
    }
    else {
        return m_write_callback;
    }

}

void FdEvent::listen(TriggerEvent event_type, std::function<void()> callback) {
    if(event_type == TriggerEvent::IN_EVENT) {
        m_listen_events.events |= EPOLLIN;
        m_read_callback = callback;    
    }
    else {
        m_listen_events.events |= EPOLLOUT;
        m_write_callback = callback;
    }
    m_listen_events.data.ptr = this;
}

void FdEvent::cancel(TriggerEvent event_type) {
    INFOLOG("cancel func arrive");
    
    if(event_type == TriggerEvent::IN_EVENT) {
        INFOLOG("cancel func in_event arrive");
        m_listen_events.events &= (~EPOLLIN); 
    }
    else {
        INFOLOG("cancel func out_event arrive");
        m_listen_events.events &= (~EPOLLOUT);
    }
    exit(0);
}

void FdEvent::setNonBlock() {
    int flag = fcntl(m_fd, F_GETFL, 0);
    if(flag & O_NONBLOCK) {
        return;
    }
    fcntl(m_fd, F_SETFL, flag | O_NONBLOCK);
}

}