#ifndef MYRPC_NET_TCP_TCP_ACCEPTOR_H
#define MYRPC_NET_TCP_TCP_ACCEPTOR_H

#include "myRPC/net/tcp/net_addr.h"

namespace myRPC
{

class TcpAcceptor {

public:
    TcpAcceptor(NetAddr::s_ptr local_addr);

    ~TcpAcceptor();

    int accept();

private:
    NetAddr::s_ptr m_local_addr; // server listen address(that bind function bound): addr->IP:PORT

    int m_family {-1};
    
    int m_listenfd {-1}; // listenfd


};
    
} // namespace myRPC


#endif