#ifndef MYRPC_NET_TCP_TCP_CONNECTION_H
#define MYRPC_NET_TCP_TCP_CONNECTION_H

#include "myRPC/net/tcp/net_addr.h"
#include "myRPC/net/tcp/tcp_buffer.h"
#include "myRPC/net/io_thread.h"


namespace myRPC
{

class TcpConnection {
public:
    enum TcpState {
        NotConnected = 1,
        Connected = 2,
        HalfClosing = 3, //
        Closed = 4,
    };

public:
    TcpConnection(IOThread* io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr);
    ~TcpConnection();

    void onRead();

    void execute();

    void onWrite();

private:

    NetAddr::s_ptr m_local_addr;
    NetAddr::s_ptr m_peer_addr;

    TcpBuffer::s_ptr m_in_buffer;  // read buffer
    TcpBuffer::s_ptr m_out_buffer; // write buffer

    IOThread* m_io_thread {nullptr}; // The I/O thread that holding this TCP connection.

    FdEvent* m_fd_event {nullptr};

    TcpState m_state;

    int m_fd {0};

};
    
} // namespace myRPC


#endif