#include "myRPC/net/tcp/tcp_server.h"
#include "myRPC/net/eventloop.h"
#include "myRPC/common/log.h"
#include "myRPC/net/tcp/tcp_connection.h"
#include "myRPC/common/config.h"

namespace myRPC
{
TcpServer::TcpServer(NetAddr::s_ptr local_addr) : m_local_addr(local_addr) {

    init();

    INFOLOG("myRPC TcpServer listen success on [%s]", m_local_addr->toString().c_str());
}

TcpServer::~TcpServer(){
    if(m_main_event_loop) {
        delete m_main_event_loop;
        m_main_event_loop = nullptr;
    }
}

void TcpServer::init() {
    // point to TcpAcceptor, and (m_local_addr) is the parameter of the constructor!!!
    m_acceptor = std::make_shared<TcpAcceptor>(m_local_addr); 

    m_main_event_loop = Eventloop::GetCurrentEventLoop();
    m_io_thread_group = new IOThreadGroup(Config::GetGlobalConfig()->m_io_threads);
    m_listen_fd_event = new FdEvent(m_acceptor->getListenFd());

    // "std::bind" is different from "bind" in socket programming
    m_listen_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpServer::onAccept, this));

    m_main_event_loop->addEpollEvent(m_listen_fd_event);
}

void TcpServer::onAccept() {
    auto re = m_acceptor->accept();
    int client_fd = re.first;
    NetAddr::s_ptr peer_addr = re.second;

    m_client_counts ++;

    // add client_fd to either IO thread
    IOThread* io_thread = m_io_thread_group->getIOThread();
    TcpConnection::s_ptr connection = std::make_shared<TcpConnection>(io_thread->getEventloop(), client_fd, 128, m_local_addr, peer_addr);
    connection->setState(Connected);
    m_client.insert(connection); // ??? why not destructor

    INFOLOG("TcpServer successfully get client fd=%d", client_fd);
}


void TcpServer::start() {
    m_io_thread_group->start();
    m_main_event_loop->loop();
}


} // namespace myRPC
