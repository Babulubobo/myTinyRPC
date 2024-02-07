#ifndef MYRPC_NET_STRING_CODER_H
#define MYRPC_NET_STRING_CODER_H

#include <string>
#include "myRPC/net/coder/abstract_coder.h"
#include "myRPC/net/coder/abstract_protocol.h"



namespace myRPC
{

class StringProtocol : public AbstractProtocol {

public:
    std::string info;

};

class StringCoder : public AbstractCoder {
    // Convert the message object into a byte stream and write it to the buffer.
    void encode(std::vector<AbstractProtocol::s_ptr>& messages, TcpBuffer::s_ptr out_buffer) {
        // std::string msg = "encode hello my rpc";
        for(size_t i = 0; i < messages.size(); i ++) {
            std::shared_ptr<StringProtocol> msg = std::dynamic_pointer_cast<StringProtocol>(messages[i]);
            out_buffer->writeToBuffer(msg->info.c_str(), msg->info.length());
        }
        
    }

    //  Convert the byte stream inside the buffer into a message object.
    void decode(std::vector<AbstractProtocol::s_ptr>& out_messages, TcpBuffer::s_ptr buffer) {
        std::vector<char> re;
        buffer->readFromBuffer(re, buffer->readAble());

        std::string info;
        for(size_t i = 0; i < re.size(); i ++) {
            info += re[i];
        }

        std::shared_ptr<StringProtocol> msg = std::make_shared<StringProtocol>();
        msg->info = info;
        msg->m_msg_id = "123456";
        out_messages.push_back(msg);
    }


};
} // namespace myRPC



#endif