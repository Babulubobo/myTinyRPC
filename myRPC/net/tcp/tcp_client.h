#ifndef MYRPC_NET_TCP_TCP_CLIENT_H
#define MYRPC_NET_TCP_TCP_CLIENT_H

#include <memory>
#include "myRPC/net/tcp/net_addr.h"
#include "myRPC/net/eventloop.h"
#include "myRPC/net/coder/abstract_protocol.h"
#include "myRPC/net/tcp/tcp_connection.h"

namespace myRPC
{

class TcpClient {

public:

    typedef std::shared_ptr<TcpClient> s_ptr;

    TcpClient(NetAddr::s_ptr peer_addr);

    ~TcpClient();

    // asyc connect
    // if connect succeed, "done" will be done
    void connect(std::function<void()> done);

    // asyc send message
    // if send message succeed, "done" will be done, "done"'s parameter is the message
    void writeMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done);

    // asyc read message
    // if read message succeed, "done" will be done, "done"'s parameter is the message
    void readMessage(const std::string& msg_id, std::function<void(AbstractProtocol::s_ptr)> done);

    void stop();

private:

    NetAddr::s_ptr m_peer_addr;

    Eventloop* m_event_loop {nullptr};

    int m_fd {-1};

    FdEvent* m_fd_event {nullptr};

    TcpConnection::s_ptr m_connection;

    

};


} // namespace myRPC


#endif