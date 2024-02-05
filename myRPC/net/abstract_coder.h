#ifndef MYRPC_NET_ABSTRACT_CODER_H
#define MYRPC_NET_ABSTRACT_CODER_H

#include <vector>
#include "myRPC/net/tcp/tcp_buffer.h"
#include "myRPC/net/abstract_protocol.h"

namespace myRPC
{

class AbstractCoder {

public:
    // Convert the message object into a byte stream and write it to the buffer.
    virtual void encode(std::vector<AbstractProtocol*>& messages, TcpBuffer::s_ptr out_buffer) = 0;

    //  Convert the byte stream inside the buffer into a message object.
    virtual void decode(std::vector<AbstractProtocol*>& messages, TcpBuffer::s_ptr buffer) = 0;

    virtual ~AbstractCoder() {};

};
    
} // namespace myRPC


#endif