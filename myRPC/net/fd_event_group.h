#ifndef MYRPC_NET_FD_EVENT_GROUP_H
#define MYRPC_NET_FD_EVENT_GROUP_H

#include "myRPC/net/fd_event.h"
#include "myRPC/common/mutex.h"
#include <vector>

namespace myRPC
{

class FdEventGroup {

public:

    FdEventGroup(int size);

    ~FdEventGroup();

    FdEvent* getFdEvent(int fd);

public:
    static FdEventGroup* GetFdEventGroup();

private:

    int m_size {0};

    std::vector<FdEvent*> m_fd_group;

    Mutex m_mutex;

};
    
} // namespace myRPC



#endif