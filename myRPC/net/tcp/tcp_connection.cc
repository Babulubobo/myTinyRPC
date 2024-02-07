#include <unistd.h>
#include "myRPC/net/tcp/tcp_connection.h"
#include "myRPC/net/fd_event_group.h"
#include "myRPC/common/log.h"
#include "myRPC/net/coder/string_coder.h"
#include "myRPC/net/coder/tinypb_coder.h"

namespace myRPC
{
TcpConnection::TcpConnection(Eventloop* event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr, TcpConnectionType type /*= TcpConnectionByServer*/) 
    : m_event_loop(event_loop), m_peer_addr(peer_addr), m_state(NotConnected), m_fd(fd), m_connection_type(type) {
    
    m_in_buffer = std::make_shared<TcpBuffer> (buffer_size);
    m_out_buffer = std::make_shared<TcpBuffer> (buffer_size);

    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(fd);
    m_fd_event->setNonBlock();

    m_coder = new TinyPBCoder();

    if(m_connection_type == TcpConnectionByServer) {
        listenRead();
        m_dispatcher = std::make_shared<RpcDispatcher>();
    }
}

TcpConnection::~TcpConnection() {
    DEBUGLOG("~TcpConnection");
    if(m_coder) {
        delete m_coder;
        m_coder = nullptr;
    }
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
        return;
    }

    if(!is_read_all) {
        ERRORLOG("not read all data");
    }

    // TODO: simply echo, next: supplementing the parsing of the RPC protocol
    execute();
}

void TcpConnection::execute() {
    if(m_connection_type == TcpConnectionByServer) {
        std::vector<myRPC::AbstractProtocol::s_ptr> result;
        std::vector<myRPC::AbstractProtocol::s_ptr> replay_message;
        m_coder->decode(result, m_in_buffer);

        for(size_t i = 0; i < result.size(); i ++) {
            // For each request, invoke the RPC method to retrieve the response message.
            // Put the response message to the out buffer, and listen write event.
            INFOLOG("success get request[%s] from client[%s]", result[i]->m_req_id.c_str(), m_peer_addr->toString().c_str());

            std::shared_ptr<TinyPBProtocal> message = std::make_shared<TinyPBProtocal>();
            // message->m_pb_data = "hello, this is my rpc test data";
            // message->m_req_id = result[i]->m_req_id;
            m_dispatcher->dispatch(result[i], message);
            replay_message.emplace_back(message);
        }

        m_coder->encode(replay_message, m_out_buffer);
        
        listenWrite();
    }
    else {
        // Decode from the buffer to obtain a message object,  and do the callback func.
        std::vector<AbstractProtocol::s_ptr> result;
        m_coder->decode(result, m_in_buffer);

        for(size_t i = 0; i < result.size(); i ++) {
            std::string req_id = result[i]->m_req_id;
            auto it = m_read_dones.find(req_id);
            if(it != m_read_dones.end()) {
                it->second(result[i]);
            }

        }
    }
    
}

void TcpConnection::onWrite() {
    // Send all bytes in out_buffer to client

    if(m_state != Connected) {
        ERRORLOG("OnWrite error: client has already disconnected, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
        return;
    }

    if(m_connection_type == TcpConnectionByClient) {
        // 1. encode the message to get the bytestream
        // 2. write the data to buffer, and send all
        
        std::vector<AbstractProtocol::s_ptr> messages;
        for(size_t i = 0; i < m_write_dones.size(); i ++) {
            messages.push_back(m_write_dones[i].first); // get 方法 ???
        }
        m_coder->encode(messages, m_out_buffer);
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
        m_fd_event->cancel(FdEvent::OUT_EVENT);
        m_event_loop->addEpollEvent(m_fd_event);
    }

    if(m_connection_type == TcpConnectionByClient) {
        for(size_t i = 0; i < m_write_dones.size(); i ++) {
            m_write_dones[i].second(m_write_dones[i].first);
        }
        m_write_dones.clear();
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

    m_fd_event->cancel(FdEvent::IN_EVENT);
    m_fd_event->cancel(FdEvent::OUT_EVENT);

    m_event_loop->delEpollEvent(m_fd_event);

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

void TcpConnection::setConnectionType(TcpConnectionType type) {
    m_connection_type = type;
}


void TcpConnection::listenWrite() {
    m_fd_event->listen(FdEvent::OUT_EVENT, std::bind(&TcpConnection::onWrite, this));
    m_event_loop->addEpollEvent(m_fd_event);
}

void TcpConnection::listenRead() {
    m_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::onRead, this));
    m_event_loop->addEpollEvent(m_fd_event);
}

void TcpConnection::pushSendMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done) {
    m_write_dones.push_back(std::make_pair(message, done));
}

void TcpConnection::pushReadMessage(const std::string& req_id, std::function<void(AbstractProtocol::s_ptr)> done) {
    m_read_dones.insert(std::make_pair(req_id, done));
}


} // namespace myRPC
