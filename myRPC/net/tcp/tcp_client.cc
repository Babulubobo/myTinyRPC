#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "myRPC/net/tcp/tcp_client.h"
#include "myRPC/common/log.h"
#include "myRPC/net/eventloop.h"
#include "myRPC/net/fd_event_group.h"


namespace myRPC
{
TcpClient::TcpClient(NetAddr::s_ptr peer_addr) : m_peer_addr(peer_addr) {
    m_event_loop = Eventloop::GetCurrentEventLoop();
    m_fd = socket(peer_addr->getFamily(), SOCK_STREAM, 0);

    if(m_fd < 0) {
        ERRORLOG("TcpClient::TcpClient() error, failed to create fd");
        return;
    }

    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(m_fd);
    m_fd_event->setNonBlock();

    m_connection = std::make_shared<TcpConnection>(m_event_loop, m_fd, 128, nullptr, m_peer_addr, TcpConnectionByClient);
    m_connection->setConnectionType(TcpConnectionByClient);
}

TcpClient::~TcpClient() {
    DEBUGLOG("TcpClient::~TcpClient()");
    if(m_fd  > 0) {
        close(m_fd);
    }
}

// asyc connect
// if connect succeed, "done" will be done
void TcpClient::connect(std::function<void()> done) {

    // connect return 0: connect success
    // connect return -1 && errno = EINPROGRESS: connection is building, add to epoll to listen write event
    //      when write event ready, call getsockopt to get the errno, if errno = 0 means connection success
    // connect return others: error
    int rt = ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockLen());
    if(rt == 0) {
        DEBUGLOG("connect [%s] success", m_peer_addr->toString().c_str());
        if(done) {
            done();
        }
    }
    else if (rt == -1) {
        if(errno == EINPROGRESS) {
            // epoll listen write event and judge errno  epoll监听可写事件，然后判断错误码
            m_fd_event->listen(FdEvent::OUT_EVENT, [this, done]() {
                int error = 0;
                socklen_t error_len = sizeof(error);
                // ???
                getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &error, &error_len);
                bool is_connect_succ = false;
                if(error == 0) {
                    DEBUGLOG("connect [%s] success", m_peer_addr->toString().c_str());
                    is_connect_succ = true;
                    m_connection->setState(Connected);
                }
                else {
                    ERRORLOG("connect error, errno=%d, error=%s", errno, strerror(errno));
                }
                m_fd_event->cancel(FdEvent::OUT_EVENT); // after connect cancel out event listen, if not it will keeps triggering
                m_event_loop->addEpollEvent(m_fd_event); // ??? why add? change the state

                // if connect succeed, do the callback function "done"
                if(is_connect_succ && done) {
                    done();
                }

            });
            m_event_loop->addEpollEvent(m_fd_event);

            if(!m_event_loop->isLooping()) {
                m_event_loop->loop();
            }
        }
        else {
            ERRORLOG("connect error, errno=%d, error=%s", errno, strerror(errno));
        }
    }
}

// asyc send message
// if send message succeed, "done" will be done, "done"'s parameter is the message
void TcpClient::writeMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done) {
    // 1. write the "message" and "done" to the connection's buffer
    // 2. start connection's write event
    m_connection->pushSendMessage(message, done);
    m_connection->listenWrite();
}

// asyc read message
// if read message succeed, "done" will be done, "done"'s parameter is the message
void TcpClient::readMessage(const std::string& msg_id, std::function<void(AbstractProtocol::s_ptr)> done) {
    // 1. Listen for the read event.
    // 2. Decode from the buffer to obtain a message object, Check if msg_id is equal, if so, it succeeds and do the callback func.
    m_connection->pushReadMessage(msg_id, done);
    m_connection->listenRead();    
}

void TcpClient::stop() {
    if(m_event_loop->isLooping()) {
        m_event_loop->stop();
    }
}


} // namespace myRPC
