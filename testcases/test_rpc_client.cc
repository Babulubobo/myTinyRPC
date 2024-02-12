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
#include "myRPC/net/rpc/rpc_channel.h"
#include "myRPC/net/rpc/rpc_controller.h"
#include "myRPC/net/rpc/rpc_closure.h"

#include <iostream>

void test_tcp_client() {
    myRPC::IPNetAddr::s_ptr addr = std::make_shared<myRPC::IPNetAddr>("127.0.0.1", 12345);
    myRPC::TcpClient client(addr);
    client.connect([addr, &client]() { // &client???
        DEBUGLOG("connect to [%s] success", addr->toString().c_str());
        std::shared_ptr<myRPC::TinyPBProtocal> message = std::make_shared<myRPC::TinyPBProtocal>();
        message->m_msg_id = "99998888";
        message->m_pb_data = "test pb data";

        makeOrderRequest request;
        request.set_price(100);
        request.set_goods("apple");

        if(!request.SerializeToString(&(message->m_pb_data))) {
            ERRORLOG("serialize error");
            return;
        }

        message->m_method_name = "Order.makeOrder";

        client.writeMessage(message, [request](myRPC::AbstractProtocol::s_ptr done) {
            DEBUGLOG("send message success, request[%s]", request.ShortDebugString().c_str());
        });
        
        client.readMessage("99998888", [](myRPC::AbstractProtocol::s_ptr done) {
            // ???: dynamic_pointer_cast: used to change base class ptr to derived class ptr (both are shared ptr)
            std::shared_ptr<myRPC::TinyPBProtocal> message = std::dynamic_pointer_cast<myRPC::TinyPBProtocal>(done);
            DEBUGLOG("msg_id [%s], get response %s", message->m_msg_id.c_str(), message->m_pb_data.c_str());
            makeOrderResponse response;

            if(!response.ParseFromString(message->m_pb_data)) {
                ERRORLOG("deserialize error");
                return;
            }

            DEBUGLOG("get response success, response [%s]", response.ShortDebugString().c_str());
        });

    });
}

void test_rpc_channel() {
    NEWRPCCHANNEL("127.0.0.1:12345", channel);

    NEWMESSAGE(makeOrderRequest, request);
    NEWMESSAGE(makeOrderResponse, response);

    request->set_price(100);
    request->set_goods("apple");

    NEWRPCCONTROLLER(controller);
    controller->SetMsgID("99998888");
    controller->SetTimeOut(10000);

    std::shared_ptr<myRPC::RpcClosure> closure = std::make_shared<myRPC::RpcClosure>([request, response, channel, controller]() mutable {
        if(controller->GetErrorCode() == 0) {
            INFOLOG("call rpc success, request[%s], response[%s]", request->ShortDebugString().c_str(), response->ShortDebugString().c_str());
            // 执行业务逻辑

        }
        else {
            ERRORLOG("call rpc failed, request[%s], error code[%d], error info[%s]", request->ShortDebugString().c_str(), 
                controller->GetErrorCode(), controller->GetErrorinfo().c_str());
        }
        INFOLOG("now exit eventloop");
        // channel->getTcpClient()->stop();
        channel.reset();
    });

    CALLRPC("127.0.0.1:12345", Order_Stub, makeOrder, controller, request, response, closure);
}


int main() {

    myRPC::Config::SetGlobalConfig(NULL);
    
    myRPC::Logger::InitGlobalLogger(0);

    // test_tcp_client();

    test_rpc_channel();

    INFOLOG("test_rpc_channel end");

    return 0;
}