#ifndef MYRPC_NET_IO_THREAD_GROUP_H
#define MYRPC_NET_IO_THREAD_GROUP_H

#include <vector>
#include "myRPC/common/log.h"
#include "myRPC/net/io_thread.h"

namespace myRPC
{
    
class IOThreadGroup {

public:

    IOThreadGroup(int size);

    ~IOThreadGroup();

    void start();

    void join();

    IOThread* getIOThread();

private:

    int m_size {0};

    std::vector<IOThread*> m_io_thread_groups;

    int m_index {0};

};


} // namespace myRPC



#endif