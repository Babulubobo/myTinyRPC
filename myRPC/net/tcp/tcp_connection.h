#ifndef MYRPC_NET_TCP_TCP_CONNECTION_H
#define MYRPC_NET_TCP_TCP_CONNECTION_H

#include <memory>
#include <map>
#include <queue>
#include "myRPC/net/tcp/net_addr.h"
#include "myRPC/net/tcp/tcp_buffer.h"
#include "myRPC/net/io_thread.h"
#include "myRPC/net/coder/abstract_protocol.h"
#include "myRPC/net/coder/abstract_coder.h"
#include "myRPC/net/rpc/rpc_dispatcher.h"


namespace myRPC
{

enum TcpState {
    NotConnected = 1,
    Connected = 2,
    HalfClosing = 3, //
    Closed = 4,
};

enum TcpConnectionType {
    TcpConnectionByServer = 1, // use in server, means connect to a client 
    TcpConnectionByClient = 2, // use in client, means connect to server
};

class TcpConnection {
public:
    typedef std::shared_ptr<TcpConnection> s_ptr;

public:
    TcpConnection(Eventloop* event_loop, int fd, int buffer_size, NetAddr::s_ptr local_addr, NetAddr::s_ptr peer_addr, TcpConnectionType type = TcpConnectionByServer);
    ~TcpConnection();

    void onRead();

    void execute();

    void onWrite();

    void setState(const TcpState state);

    TcpState getState();

    void clear();

    void shutdown(); // The server actively closes the connection.

    void setConnectionType(TcpConnectionType type);

    // start listen write event(used in tcp client)
    void listenWrite();

    // start listen read event(used in tcp client)
    void listenRead();

    void pushSendMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done);

    void pushReadMessage(const std::string& req_id, std::function<void(AbstractProtocol::s_ptr)> done);

    NetAddr::s_ptr getLocalAddr();

    NetAddr::s_ptr getPeerAddr();

private:

    Eventloop* m_event_loop {nullptr}; // The I/O thread that holding this TCP connection.

    NetAddr::s_ptr m_local_addr;
    NetAddr::s_ptr m_peer_addr;

    TcpBuffer::s_ptr m_in_buffer;  // read buffer
    TcpBuffer::s_ptr m_out_buffer; // write buffer


    FdEvent* m_fd_event {nullptr};

    AbstractCoder* m_coder {nullptr};

    TcpState m_state;

    int m_fd {0};

    TcpConnectionType m_connection_type {TcpConnectionByServer};

    // std::pair<AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)>>
    std::vector<std::pair<AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)>>> m_write_dones;

    // key: req_id
    std::map<std::string, std::function<void(AbstractProtocol::s_ptr)>> m_read_dones;

    std::shared_ptr<RpcDispatcher> m_dispatcher;

};
    
} // namespace myRPC


#endif