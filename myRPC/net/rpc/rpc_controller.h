#ifndef MYRPC_NET_RPC_RPC_CONTROLLER_H
#define MYRPC_NET_RPC_RPC_CONTROLLER_H

#include <string>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include <myRPC/net/tcp/net_addr.h>

namespace myRPC
{

class RpcController : public google::protobuf::RpcController {

public:
    RpcController(){}
    ~RpcController(){}

    void Reset();

    bool Failed() const;

    std::string ErrorText() const;

    void StartCancel();

    void SetFailed(const std::string& reason);

    bool IsCanceled() const;

    void NotifyOnCancel(google::protobuf::Closure* callback);

    void SetError(int32_t error_code, const std::string error_info);

    int32_t GetErrorCode();

    std::string GetErrorinfo();

    void SetMsgID(const std::string& msg_id);

    std::string GetMsgID();

    void SetLocalAddr(NetAddr::s_ptr addr);

    void SetPeerAddr(NetAddr::s_ptr addr);

    NetAddr::s_ptr GetLocalAddr();

    NetAddr::s_ptr GetPeerAddr();

    void SetTimeOut(int timeout);
    
    int GetTimeOut();

private:
    int32_t m_error_code {0};
    std::string m_error_info;
    std::string m_msg_id;

    bool m_is_failed {false};
    bool m_is_cancel {false};

    NetAddr::s_ptr m_local_addr;
    NetAddr::s_ptr m_peer_addr;

    int m_timeout {1000}; //ms

};
    
} // namespace myRPC


#endif