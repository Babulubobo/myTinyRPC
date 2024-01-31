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
    m_fd_event->setNonBlock();
    m_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::onRead, this));

    io_thread->getEventloop()->addEpollEvent(m_fd_event);
}

TcpConnection::~TcpConnection() {
    DEBUGLOG("~TcpConnection");
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
        DEBUGLOG("success read %d bytes from addr[%s], client fd[%d]", rt, m_peer_addr->toString().c_str(), m_fd);
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
        else if (rt == 0) {
            is_close = true;
            break;
        }
        else if (rt == -1 && errno == EAGAIN) {
            is_read_all = true;
            break;
        }
    }

    if(is_close) {
        INFOLOG("peer closed, peer addr [%s] ,clientfd [%d]", m_peer_addr->toString().c_str(), m_fd);
        clear();
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
    for(size_t i = 0; i < tmp.size(); i ++){
        msg += tmp[i];
    }

    INFOLOG("success get request[%s] from client[%s]", msg.c_str(), m_peer_addr->toString().c_str());

    m_out_buffer->writeToBuffer(msg.c_str(), msg.length());
    
    m_fd_event->listen(FdEvent::OUT_EVENT, std::bind(&TcpConnection::onWrite, this));
    m_io_thread->getEventloop()->addEpollEvent(m_fd_event);
}

void TcpConnection::onWrite() {
    // Send all bytes in out_buffer to client

    if(m_state != Connected) {
        ERRORLOG("OnWrite error: client has already disconnected, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
        return;
    }

    bool is_write_all = false;
    while(true) {
        if(m_out_buffer->readAble() == 0) {
            DEBUGLOG("no data need to sent to client [%s]", m_peer_addr->toString().c_str());
            is_write_all = true;
            break;
        }

        int write_size = m_out_buffer->readAble();
        int read_index = m_out_buffer->readIndex();
        int rt = write(m_fd, &(m_out_buffer->m_buffer[read_index]), write_size);

        if(rt >= write_size) {
            DEBUGLOG("no data need to sent to client [%s]", m_peer_addr->toString().c_str());
            is_write_all = true;
            break;
        }
        if(rt == -1 && errno == EAGAIN) {
            // buffer is full, can't write more
            // send bytes until next time the fd can write
            ERRORLOG("write data error, errno==EAGAIN and rt==-1");
            break;
        }
    }
    if(is_write_all) {
        INFOLOG("is_write_all true");
        m_fd_event->cancel(FdEvent::OUT_EVENT);
        m_io_thread->getEventloop()->addEpollEvent(m_fd_event);
    }
}

void TcpConnection::setState(const TcpState state) {
    m_state = state; //??? m_state = Connected ???
}

TcpState TcpConnection::getState() {
    return m_state;
}

void TcpConnection::clear() {
    // make some clear when connection is close

    if(m_state == Closed) {
        return;
    }

    m_io_thread->getEventloop()->delEpollEvent(m_fd_event);

    m_state = Closed;
}

void TcpConnection::shutdown() {
    if(m_state == Closed || m_state == NotConnected) {
        return;
    }

    m_state = HalfClosing;

    // Calling shutdown to close both reading and writing 
    // means that the server will no longer perform read or write operations on this fd.
    // send FIN signal, trigger 4 times wave hand section 1
    // when fd read event happen and read data=0, means peer send FIN signal
    ::shutdown(m_fd, SHUT_RDWR);
}

} // namespace myRPC
