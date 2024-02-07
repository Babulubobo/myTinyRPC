#include <assert.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>
#include <unistd.h>
#include <google/protobuf/service.h>
#include "myRPC/common/log.h"
#include "myRPC/common/config.h"
#include "myRPC/net/tcp/tcp_client.h"

#include "myRPC/net/tcp/net_addr.h"
#include "myRPC/net/tcp/tcp_server.h"
#include "myRPC/net/coder/string_coder.h"
#include "myRPC/net/coder/abstract_protocol.h"
#include "myRPC/net/coder/tinypb_coder.h"
#include "myRPC/net/coder/tinypb_protocol.h"
#include "order.pb.h"
#include "myRPC/net/rpc/rpc_dispatcher.h"

#include <iostream>

class OrderImpl : public Order {
public:
    virtual void makeOrder(google::protobuf::RpcController* controller,
                        const ::makeOrderRequest* request,
                        ::makeOrderResponse* response,
                        ::google::protobuf::Closure* done) {
        
        if(request->price() < 10) {
            response->set_ret_code(-1);
            response->set_res_info("short balance");
            return;
        }
        response->set_order_id("20240207");

    }
};

void test_tcp_server() {
    myRPC::IPNetAddr::s_ptr addr = std::make_shared<myRPC::IPNetAddr>("127.0.0.1", 12345);
    DEBUGLOG("create addr %s", addr->toString().c_str());

    myRPC::TcpServer tcp_server(addr);

    tcp_server.start();
}

int main() {

    myRPC::Config::SetGlobalConfig("../conf/myRPC.xml");
    
    myRPC::Logger::InitGlobalLogger();

    std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();
    myRPC::RpcDispatcher::GetRpcDispatcher()->registerService(service);

    test_tcp_server();

    return 0;
}