#ifndef MYRPC_NET_RPC_RPC_DISPATCHER_H
#define MYRPC_NET_RPC_RPC_DISPATCHER_H

#include <map>
#include <string>
#include <memory>
#include "myRPC/net/coder/abstract_protocol.h"
#include "google/protobuf/service.h"
#include "myRPC/net/coder/tinypb_protocol.h"

/*
RPC服务端流程:

启动的时候就注册 OrderService 对象

1. 从 buffer 中读取数据，然后 encode 得到请求的 TinyPBProtocol 对象，然后从请求的 TinyPBProtocol 得到 method_name,
    从 OrderService 对象里根据 service.method_name 找到方法 func
2. 找到对应的 request type 以及 response type
3. 将请求体 TinyPBProtocol 里面的 pb_data 反序列化为 request type 的一个对象，声明一个空的 response type 对象
4. func(request, response)
5. 将 response 对象序列化为 pb_data, 再塞入到 TinyPBProtocol 结构体中。做 encode 然后塞入到 buffer 里，即可发送回包。



*/

namespace myRPC
{

class TcpConnection;

class RpcDispatcher {

public:

    static RpcDispatcher* GetRpcDispatcher();

public:

    typedef std::shared_ptr<google::protobuf::Service> service_s_ptr;

    void dispatch(AbstractProtocol::s_ptr request, AbstractProtocol::s_ptr response, TcpConnection* connection);

    void registerService(service_s_ptr service);

    void setTinyPBError(std::shared_ptr<TinyPBProtocal> msg, int32_t err_code, const std::string err_info);

private:
    bool parseServiceFullName(const std::string& full_name, std::string& service_name, std::string& method_name);

private:
    std::map<std::string, service_s_ptr> m_service_map;

};
    
} // namespace myRPC


#endif