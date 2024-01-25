#ifndef MYRPC_NET_WAKEUP_FDEVENT_H
#define MYRPC_NET_WAKEUP_FDEVENT_H

#include "myRPC/net/fd_event.h"
#include "myRPC/common/log.h"

namespace myRPC {

class WakeUpFdEvent : public FdEvent{
public:
    WakeUpFdEvent(int fd);
    ~WakeUpFdEvent();

    void wakeup();
private:

};

}

#endif