#ifndef MYRPC_NET_ABSTRACT_PROTOCOL_H
#define MYRPC_NET_ABSTRACT_PROTOCOL_H

#include <memory>

#include "myRPC/net/tcp/tcp_buffer.h"

namespace myRPC
{

class AbstractProtocol : public std::enable_shared_from_this<AbstractProtocol> { // ??? yingyong tiaojian???

public:
    typedef std::shared_ptr<AbstractProtocol> s_ptr;

    std::string getReqID() {
        return m_req_id;
    }

    void setReqID(const std::string& req_id) {
        m_req_id = req_id;
    }

    virtual ~AbstractProtocol() {};

protected:
    std::string m_req_id; // request number, unique

};


    
} // namespace myRPC


#endif