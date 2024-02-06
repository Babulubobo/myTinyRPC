#ifndef MYRPC_NET_CODER_TINYPB_CODER_H
#define MYRPC_NET_CODER_TINYPB_CODER_H

#include "myRPC/net/coder/abstract_coder.h"

namespace myRPC
{

class TinyPBCoder : public AbstractCoder {

public:
    // Convert the message object into a byte stream and write it to the buffer.
    void encode(std::vector<AbstractProtocol::s_ptr>& messages, TcpBuffer::s_ptr out_buffer);

    //  Convert the byte stream inside the buffer into a message object.
    void decode(std::vector<AbstractProtocol::s_ptr>& out_messages, TcpBuffer::s_ptr buffer);

private:
    const char* encodeTinyPB(std::shared_ptr<TinyPBProtocal> message, int& len);
};
    
} // namespace myRPC


#endif