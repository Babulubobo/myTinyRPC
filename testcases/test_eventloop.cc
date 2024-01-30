#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <memory>
#include "myRPC/common/log.h"
#include "myRPC/common/config.h"
#include "myRPC/net/eventloop.h"
#include "myRPC/net/timer_event.h"
#include "myRPC/net/io_thread.h"

void test_io_thread() {

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1) {
        ERRORLOG("listenfd  = -1");
        exit(0);
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_port = htons(12366);
    addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &addr.sin_addr);

    int rt = bind(listenfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if(rt != 0) {
        ERRORLOG("bind error");
        exit(0);
    }

    listen(listenfd, 100);
    if(rt != 0) {
        ERRORLOG("listen error");
        exit(0);
    }

    myRPC::FdEvent event(listenfd);
    event.listen(myRPC::FdEvent::IN_EVENT, [listenfd](){
        sockaddr_in peer_addr;
        socklen_t addr_len = sizeof(peer_addr);
        memset(&peer_addr, 0, sizeof(peer_addr));
        int clientfd = accept(listenfd, reinterpret_cast<sockaddr*>(&peer_addr), &addr_len);

        inet_ntoa(peer_addr.sin_addr);
        DEBUGLOG("success get client fd[%d], peer addr: [%s:%d]", clientfd, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
    });


    int i = 0;
    myRPC::TimerEvent::s_ptr timer_event = std::make_shared<myRPC::TimerEvent>(
        1000, true, [&i]() {
            INFOLOG("trigger timer event, count = %d", i++);
        }
    );

    myRPC::IOThread io_thread; // stack, when function over it will ~IOThread

    io_thread.getEventloop()->addEpollEvent(&event);
    io_thread.getEventloop()->addTimerEvent(timer_event);
    io_thread.start();

    io_thread.join();
}


int main() {
    myRPC::Config::SetGlobalConfig("../conf/myRPC.xml");

    myRPC::Logger::InitGlobalLogger();

    test_io_thread();

    // myRPC::Eventloop* eventloop = new myRPC::Eventloop();

    // int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    // if(listenfd == -1) {
    //     ERRORLOG("listenfd  = -1");
    //     exit(0);
    // }

    // sockaddr_in addr;
    // memset(&addr, 0, sizeof(addr));
    // addr.sin_port = htons(12399);
    // addr.sin_family = AF_INET;
    // inet_aton("127.0.0.1", &addr.sin_addr);

    // int rt = bind(listenfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    // if(rt != 0) {
    //     ERRORLOG("bind error");
    //     exit(0);
    // }
    // listen(listenfd, 100);
    // if(rt != 0) {
    //     ERRORLOG("listen error");
    //     exit(0);
    // }

    // myRPC::FdEvent event(listenfd);
    // event.listen(myRPC::FdEvent::IN_EVENT, [listenfd](){
    //     sockaddr_in peer_addr;
    //     socklen_t addr_len = sizeof(peer_addr);
    //     memset(&peer_addr, 0, sizeof(peer_addr));
    //     int clientfd = accept(listenfd, reinterpret_cast<sockaddr*>(&peer_addr), &addr_len);

    //     inet_ntoa(peer_addr.sin_addr);
    //     DEBUGLOG("success get client fd[%d], peer addr: [%s:%d]", clientfd, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
    // });
    
    // eventloop->addEpollEvent(&event);

    // int i = 0;
    // myRPC::TimerEvent::s_ptr timer_event = std::make_shared<myRPC::TimerEvent>(
    //     1000, true, [&i]() {
    //         INFOLOG("trigger timer event, count = %d", i++);
    //     }
    // );

    // eventloop->addTimerEvent(timer_event);

    // eventloop->loop();


    return 0;
}

