#ifndef MYRPC_NET_ABSTRACT_PROTOCOL_H
#define MYRPC_NET_ABSTRACT_PROTOCOL_H

#include <memory>
#include <string>
#include "myRPC/net/tcp/tcp_buffer.h"

namespace myRPC
{

struct AbstractProtocol : public std::enable_shared_from_this<AbstractProtocol> { // ??? yingyong tiaojian???

public:
    typedef std::shared_ptr<AbstractProtocol> s_ptr;

    virtual ~AbstractProtocol() {};

public:
    std::string m_req_id; // request number, unique

};


    
} // namespace myRPC


#endif