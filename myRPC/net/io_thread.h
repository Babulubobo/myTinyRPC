#ifndef MYRPC_NET_IO_THREAD_H
#define MYRPC_NET_IO_THREAD_H

#include <pthread.h>
#include <semaphore.h>
#include "myRPC/net/eventloop.h"

namespace myRPC
{
    
class IOThread {

public:
    IOThread();

    ~IOThread();

    Eventloop* getEventloop();
    

public:
    static void* Main(void* arg);

private:
    pid_t m_thread_id {-1};  // thread id

    pthread_t m_thread {-1}; // thread handle

    Eventloop* m_event_loop {nullptr}; // The loop object of the current I/O thread.

    sem_t m_init_semaphore;
};



} // namespace myRPC



#endif