#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include "myRPC/net/rpc/rpc_dispatcher.h"
#include "myRPC/net/coder/tinypb_protocol.h"
#include "myRPC/common/log.h"

namespace myRPC
{

void RpcDispatcher::dispatch(AbstractProtocol::s_ptr request, AbstractProtocol::s_ptr response) {
    std::shared_ptr<TinyPBProtocal> req_protocol = std::dynamic_pointer_cast<TinyPBProtocal>(request);
    std::shared_ptr<TinyPBProtocal> rsp_protocol = std::dynamic_pointer_cast<TinyPBProtocal>(response);
    std::string method_full_name = req_protocol->m_method_name;
    std::string service_name, method_name;

    if(parseServiceFullName(method_full_name, service_name, method_full_name)) {
        // TODO: 出错处理后面补充
    }
    auto it = m_service_map.find(service_name);
    if(it == m_service_map.end()) {
        // TODO
    }

    service_s_ptr service = (*it).second;
    const google::protobuf::MethodDescriptor* method =  service->GetDescriptor()->FindMethodByName(method_name);
    if(method == nullptr) {
        // TODO
    }

    google::protobuf::Message* req_msg = service->GetRequestPrototype(method).New();

    // 反序列化， 将 pb_data 反序列化为 request
    // ??? req_protocol->m_pb_data 什么时候序列化的???
    if(!req_msg->ParseFromString(req_protocol->m_pb_data)) {
        // TODO
    }

    INFOLOG("req_id[%s], get rpc request[%s]", req_protocol->m_req_id.c_str(), req_msg->ShortDebugString().c_str());

    google::protobuf::Message* rsp_msg = service->GetResponsePrototype(method).New();

    service->CallMethod(method, NULL, req_msg, rsp_msg, NULL);

    rsp_protocol->m_req_id = req_protocol->m_req_id;
    rsp_protocol->m_method_name = req_protocol->m_method_name;
    rsp_protocol->m_err_code = 0;

    rsp_msg->SerializeToString(&(rsp_protocol->m_pb_data));
}

bool RpcDispatcher::parseServiceFullName(const std::string& full_name, std::string& service_name, std::string& method_name) {
    if(full_name.empty()) {
        ERRORLOG("full name empty");
        return false;
    }
    size_t i = full_name.find_first_of(".");
    if(i == full_name.npos) { // ???
        ERRORLOG("not find . in full name [%s]", full_name.c_str());
        return false;
    }
    service_name = full_name.substr(0, i); //???
    method_name = full_name.substr(i + 1, full_name.length() - i - 1);
    INFOLOG("parse service_name[%s] and method_name[%s] from full name [%s]", service_name.c_str(), method_name.c_str(), full_name.c_str());
    return true;
}
    
void RpcDispatcher::registerService(service_s_ptr service) {
    std::string service_name = service->GetDescriptor()->full_name();
    m_service_map[service_name] = service;
}

} // namespace myRPC
