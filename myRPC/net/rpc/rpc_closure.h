#ifndef MYRPC_NET_RPC_RPC_CLOSURE_H
#define MYRPC_NET_RPC_RPC_CLOSURE_H

#include <google/protobuf/stubs/callback.h>
#include <functional>

namespace myRPC
{

class RpcClosure : public google::protobuf::Closure {

public:
    RpcClosure(std::function<void()> cb) : m_cb(cb) {};

    void Run() override {
        if(m_cb) {
            m_cb();
        }
    }

private:
    std::function<void()> m_cb {nullptr};
};
    
} // namespace myRPC


#endif