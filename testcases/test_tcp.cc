#include <memory>
#include "myRPC/common/log.h"
#include "myRPC/net/tcp/net_addr.h"
#include "myRPC/net/tcp/tcp_server.h"

void test_tcp_server() {
    myRPC::IPNetAddr::s_ptr addr = std::make_shared<myRPC::IPNetAddr>("127.0.0.1", 12345);
    DEBUGLOG("create addr %s", addr->toString().c_str());

    myRPC::TcpServer tcp_server(addr);

    tcp_server.start();
}

int main() {

    myRPC::Config::SetGlobalConfig("../conf/myRPC.xml");
    myRPC::Logger::InitGlobalLogger();

    test_tcp_server();
    
    return 0;
}