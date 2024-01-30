#include "myRPC/net/tcp/tcp_server.h"
#include "myRPC/net/eventloop.h"
#include "myRPC/common/log.h"

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
    m_io_thread_group = new IOThreadGroup(2);
    m_listen_fd_event = new FdEvent(m_acceptor->getListenFd());

    // "std::bind" is different from "bind" in socket programming
    m_listen_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpServer::onAccept, this));

    m_main_event_loop->addEpollEvent(m_listen_fd_event);
}

void TcpServer::onAccept() {
    int client_fd = m_acceptor->accept();
    // FdEvent client_fd_event(client_fd);
    m_client_counts ++;

    // TODO: add client_fd to either IO thread
    // m_io_thread_group->getIOThread()->getEventloop()->addEpollEvent(&client_fd_event);

    INFOLOG("TcpServer successfully get client fd=%d", client_fd);
}


void TcpServer::start() {
    m_io_thread_group->start();
    m_main_event_loop->loop();
}


} // namespace myRPC
