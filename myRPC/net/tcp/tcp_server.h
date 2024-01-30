#ifndef MYRPC_NET_TCP_TCP_SERVER_H
#define MYRPC_NET_TCP_TCP_SERVER_H

#include "myRPC/net/tcp/tcp_acceptor.h"
#include "myRPC/net/tcp/net_addr.h"
#include "myRPC/net/eventloop.h"
#include "myRPC/net/io_thread_group.h"

namespace myRPC
{
    
class TcpServer {

public:
    TcpServer(NetAddr::s_ptr local_addr);

    ~TcpServer();

    void start();

private:

    void init();

    // when new client connect need to do the function
    void onAccept();

private:
    TcpAcceptor::s_ptr m_acceptor;

    NetAddr::s_ptr m_local_addr; // local listen addr

    Eventloop* m_main_event_loop {nullptr}; //main Reactor

    IOThreadGroup* m_io_thread_group {nullptr}; // subReactor group

    FdEvent* m_listen_fd_event;

    int m_client_counts {0};

};


} // namespace myRPC


#endif