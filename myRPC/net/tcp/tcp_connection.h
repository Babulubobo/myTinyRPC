#ifndef MYRPC_NET_TCP_TCP_CONNECTION_H
#define MYRPC_NET_TCP_TCP_CONNECTION_H

#include <memory>
#include "myRPC/net/tcp/net_addr.h"
#include "myRPC/net/tcp/tcp_buffer.h"
#include "myRPC/net/io_thread.h"


namespace myRPC
{

enum TcpState {
    NotConnected = 1,
    Connected = 2,
    HalfClosing = 3, //
    Closed = 4,
};

class TcpConnection {
public:
    typedef std::shared_ptr<TcpConnection> s_ptr;

public:
    TcpConnection(IOThread* io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr);
    ~TcpConnection();

    void onRead();

    void execute();

    void onWrite();

    void setState(const TcpState state);

    TcpState getState();

    void clear();

    void shutdown(); // The server actively closes the connection.

private:

    IOThread* m_io_thread {nullptr}; // The I/O thread that holding this TCP connection.

    NetAddr::s_ptr m_local_addr;
    NetAddr::s_ptr m_peer_addr;

    TcpBuffer::s_ptr m_in_buffer;  // read buffer
    TcpBuffer::s_ptr m_out_buffer; // write buffer

    

    FdEvent* m_fd_event {nullptr};

    TcpState m_state;

    int m_fd {0};

};
    
} // namespace myRPC


#endif