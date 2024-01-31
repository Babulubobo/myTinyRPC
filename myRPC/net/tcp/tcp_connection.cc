#include <unistd.h>
#include "myRPC/net/tcp/tcp_connection.h"
#include "myRPC/net/fd_event_group.h"
#include "myRPC/common/log.h"

namespace myRPC
{
TcpConnection::TcpConnection(IOThread* io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr) 
    : m_io_thread(io_thread), m_peer_addr(peer_addr), m_state(NotConnected), m_fd(fd) {
    
    m_in_buffer = std::make_shared<TcpBuffer> (buffer_size);
    m_out_buffer = std::make_shared<TcpBuffer> (buffer_size);

    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(fd);
    m_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::onRead, this));
}

TcpConnection::~TcpConnection() {

}

void TcpConnection::onRead() {
    // 1. Calling the system's read function to read bytes from the socket buffer, and save into the 'in_buffer'
    if(m_state != Connected) {
        ERRORLOG("OnRead error: client has already disconnected, addr[%s], clientfd[%d]",m_peer_addr->toString().c_str(), m_fd);
        return;
    }

    bool is_read_all = false;
    bool is_close = false;

    while(!is_read_all) {
        if(m_in_buffer->writeAble() == 0) {
            m_in_buffer->resizeBuffer(2 * m_in_buffer->m_buffer.size());
        }
        int read_count = m_in_buffer->writeAble();
        int write_index = m_in_buffer->writeIndex();

        int rt = read(m_fd, &(m_in_buffer->m_buffer[write_index]), read_count);
        DEBUGLOG("success read %d bytes from addr[%s], client fd[%d]", m_peer_addr->toString().c_str(), m_fd);
        if(rt > 0) {
            m_in_buffer->moveWriteIndex(rt);

            if(rt == read_count) { // the bytes may not have been read completely 
                continue;
            }
            else if(rt < read_count) { // read completely
                is_read_all = true;
                break;
            }
        }
        else {
            is_close = true;
        }
    }

    if(is_close) {
        // TODO: handle the closure of the TCP connection
        INFOLOG("peer closed, peer addr [%s] ,clientfd [%d]", m_peer_addr->toString().c_str(), m_fd);
    }

    if(!is_read_all) {
        ERRORLOG("not read all data");
    }

    // TODO: simply echo, next: supplementing the parsing of the RPC protocol
    execute();
}

void TcpConnection::execute() {
    // Execute business logic for the RPC request, obtain the RPC response, and then send the RPC response.
    std::vector<char> tmp;
    int size = m_in_buffer->readAble();
    tmp.resize(size);
    m_in_buffer->readFromBuffer(tmp, size); //???

    std::string msg;
    for(int i = 0; i < tmp.size(); i ++){
        msg += tmp[i];
    }

    INFOLOG("success get request[%s] from client[%s]", msg.c_str(), m_peer_addr->toString().c_str());

    m_out_buffer->writeToBuffer(msg.c_str(), msg.length());
    
    m_fd_event->listen(FdEvent::OUT_EVENT, std::bind(&TcpConnection::onWrite, this));
}

void TcpConnection::onWrite() {
    // Send all bytes in out_buffer to client

    if(m_state != Connected) {
        ERRORLOG("OnWrite error: client has already disconnected, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
    }

    while(true) {
        if(m_out_buffer->readAble() == 0) {
            DEBUGLOG("no data need to sent to client [%s]", m_peer_addr->toString().c_str(), m_fd);
            break;
        }

        int write_size = m_out_buffer->readAble();
        int read_index = m_out_buffer->readIndex();
        int rt = write(m_fd, &(m_out_buffer->m_buffer[read_index]), write_size);

        if(rt >= write_size) {
            DEBUGLOG("no data need to sent to client [%s]", m_peer_addr->toString().c_str(), m_fd);
            break;
        }
        if(rt == -1 && errno == EAGAIN) {
            // buffer is full, can't write more
            // send bytes until next time the fd can write
            ERRORLOG("write data error, errno==EAGAIN and rt==-1");
            break;
        }
    }
}


} // namespace myRPC
