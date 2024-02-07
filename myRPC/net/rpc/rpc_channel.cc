#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include "myRPC/net/rpc/rpc_channel.h"
#include "myRPC/net/rpc/rpc_controller.h"
#include "myRPC/net/tcp/tcp_client.h"
#include "myRPC/net/coder/tinypb_protocol.h"
#include "myRPC/common/msg_id_util.h"
#include "myRPC/common/log.h"
#include "myRPC/common/error_code.h"

namespace myRPC
{

RpcChannel::RpcChannel(NetAddr::s_ptr peer_addr) : m_peer_addr(peer_addr) {

}

RpcChannel::~RpcChannel() {

}

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                        google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                        google::protobuf::Message* response, google::protobuf::Closure* done) {
        
    std::shared_ptr<myRPC::TinyPBProtocal> req_protocol = std::make_shared<myRPC::TinyPBProtocal>();
    RpcController* my_controller = dynamic_cast<RpcController*>(controller);

    if(my_controller == nullptr) {
        ERRORLOG("failed call method, RpcController convert error");
        return;
    }

    if(my_controller->GetMsgID().empty()) {
        req_protocol->m_msg_id = MsgIDUtil::GenMsgID();
        my_controller->SetMsgID(req_protocol->m_msg_id);
    }
    else {
        req_protocol->m_msg_id = my_controller->GetMsgID();
    }

    req_protocol->m_method_name = method->full_name();
    INFOLOG("%s | call method name [%s]", req_protocol->m_msg_id.c_str(), req_protocol->m_method_name.c_str());

    // request serialize
    if(!request->SerializeToString(&(req_protocol->m_pb_data))) {
        std::string err_info = "failed to serialize";
        my_controller->SetError(ERROR_FAILED_SERIALIZE, err_info);
        ERRORLOG("%s | %s, origin request [%s] failed to serialize", req_protocol->m_msg_id.c_str(), err_info.c_str(), request->ShortDebugString().c_str());
    }

    TcpClient client(m_peer_addr);

    client.connect([&client, req_protocol, done]() {
        client.writeMessage(req_protocol, [&client, req_protocol, done](AbstractProtocol::s_ptr) {
            INFOLOG("%s | send request success., method_name [%s]", 
                req_protocol->m_msg_id.c_str(), req_protocol->m_method_name.c_str());

            client.readMessage(req_protocol->m_msg_id, [done](AbstractProtocol::s_ptr msg) {
                std::shared_ptr<myRPC::TinyPBProtocal> rsp_protocol = std::dynamic_pointer_cast<myRPC::TinyPBProtocal>(msg);
                INFOLOG("%s | success get rpc response, call method name %s",
                     rsp_protocol->m_msg_id.c_str(), rsp_protocol->m_method_name.c_str());

                if(done) {
                    done->Run();
                }
            });
        });
    });



}
} // namespace myRPC
