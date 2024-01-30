#include <pthread.h>
#include <assert.h>

#include "myRPC/net/io_thread.h"
#include "myRPC/common/log.h"
#include "myRPC/common/util.h"

namespace myRPC
{
    
IOThread::IOThread() {

    int rt = sem_init(&m_init_semaphore, 0, 0);
    assert(rt == 0);

    rt = sem_init(&m_start_semaphore, 0, 0);
    assert(rt == 0);

    // pthread_create: 
    // para1: the new thread, para2: the states of the new thread
    // para3: the function that new thread need to do
    // para4: the parameter of the para3
    pthread_create(&m_thread, NULL, IOThread::Main, this);

    // wait until Main() finish every code before loop()(because its dead loop)
    // need to wait because the rpc is asynchronous, it's possible to access loop() before it's init
    sem_wait(&m_init_semaphore);
    DEBUGLOG("IOThread %d create success", m_thread_id);
}

IOThread::~IOThread() {

    m_event_loop->stop();
    sem_destroy(&m_init_semaphore);
    sem_destroy(&m_start_semaphore);

    pthread_join(m_thread, NULL);

    if(m_event_loop) {
        delete m_event_loop;
        m_event_loop = nullptr;
    }
}


void* IOThread::Main(void* arg) {

    IOThread* thread = static_cast<IOThread*> (arg);

    thread->m_event_loop = new Eventloop();
    thread->m_thread_id = getThreadId();

    // wake up the thread that is waiting
    sem_post(&thread->m_init_semaphore);

    // let IO thread wait until we let it start
    DEBUGLOG("IOThread %d created, wait start semaphore", thread->m_thread_id);
    sem_wait(&thread->m_start_semaphore);
    DEBUGLOG("IOThread %d start loop", thread->m_thread_id);
    thread->m_event_loop->loop();

    DEBUGLOG("IOThread %d end loop", thread->m_thread_id);

    return NULL;

}

Eventloop* IOThread::getEventloop() {
    return m_event_loop;
}

void IOThread::start() {

    DEBUGLOG("Now invoke IOThread %d", m_thread_id);
    sem_post(&m_start_semaphore);

}

void IOThread::join() {
    pthread_join(m_thread, NULL);
}


} // namespace myRPC
