#include <vector>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "myRPC/net/coder/tinypb_coder.h"
#include "myRPC/net/coder/tinypb_protocol.h"
#include "myRPC/common/util.h"
#include "myRPC/common/log.h"

namespace myRPC
{

// Convert the message object into a byte stream and write it to the buffer.
void TinyPBCoder::encode(std::vector<AbstractProtocol::s_ptr>& messages, TcpBuffer::s_ptr out_buffer) {
    for(auto &i : messages) {
        // dynamic_pointer_cast：作用和dynamic_cast相同，用于向下类型转换，只不过是用在智能指针上
        std::shared_ptr<TinyPBProtocal> msg = std::dynamic_pointer_cast<TinyPBProtocal>(i);
        int len = 0;
        const char*buf = encodeTinyPB(msg, len);

        if(buf != nullptr && len != 0) {
            out_buffer->writeToBuffer(buf, len);
        }

        if(buf) {
            free((void*)buf);
            buf = nullptr;
        }
    }
}

//  Convert the byte stream inside the buffer into a message object.
void TinyPBCoder::decode(std::vector<AbstractProtocol::s_ptr>& out_messages, TcpBuffer::s_ptr buffer) {

    while(1) {

        
        // Iterate through the buffer, find PB_START, then parse out the entire package length
        // get the position of the end delimiter, and determine if it is PB_END.
        std::vector<char> tmp = buffer->m_buffer;
        int start_index = buffer->readIndex();
        int end_index = -1;

        int pk_len = 0;
        bool parse_success = false;
        int i = 0;
        for(i = start_index; i < buffer->writeIndex(); i ++) {
            if(tmp[i] == TinyPBProtocal::PB_START) {
                // get the next 4 bytes(1 int), 网络字节序，需要转化为主机字节序
                if(i + 1 < buffer->writeIndex()) { // 1 int = 4 bytes
                    pk_len = getInt32FromNetByte(&tmp[i+1]);
                    DEBUGLOG("get pk_len = %d", pk_len);

                    int j = i + pk_len - 1; // the PB_END index
                    if(j >= buffer->writeIndex()) {
                        continue; //找到真正代表协议起始头的0x02
                    }
                    if(tmp[j] == TinyPBProtocal::PB_END) {
                        start_index = i;
                        end_index = j;
                        parse_success = true;
                        break;
                    }
                }
            }
        }

        if(i >= buffer->writeIndex()) {
            DEBUGLOG("decode end, read all buffer data");
            return;
        }

        if(parse_success) {
            buffer->moveReadIndex(end_index - start_index + 1);
            std::shared_ptr<TinyPBProtocal> message = std::make_shared<TinyPBProtocal>();
            message->m_pk_len = pk_len;

            int msg_id_len_index = start_index + sizeof(char) + sizeof(message->m_pk_len);
            if(msg_id_len_index >= end_index) {
                message->parse_success = false;
                ERRORLOG("parse error, msg_id_len_index[%d] >= end_index[%d]", msg_id_len_index, end_index);
                continue;
            }
            message->m_msg_id_len = getInt32FromNetByte(&tmp[msg_id_len_index]);
            DEBUGLOG("parse msg_id_len=%d", message->m_msg_id_len);

            int msg_id_index = msg_id_len_index + sizeof(message->m_msg_id_len);
            
            char msg_id[100] = {0};
            memcpy(&msg_id[0], &tmp[msg_id_index], message->m_msg_id_len);
            message->m_msg_id = std::string(msg_id);
            DEBUGLOG("parse msg_id=%s", message->m_msg_id.c_str());

            int method_name_len_index = msg_id_index + message->m_msg_id_len;  
            if(method_name_len_index >= end_index) {
                message->parse_success = false;
                ERRORLOG("parse error, method_name_len_index[%d] >= end_index[%d]", method_name_len_index, end_index);
                continue;
            }
            message->m_method_name_len = getInt32FromNetByte(&tmp[method_name_len_index]);

            int method_name_index = method_name_len_index + sizeof(message->m_method_name_len);
            char method_name[512] = {0};
            memcpy(&method_name[0], &tmp[method_name_index], message->m_method_name_len);
            message->m_method_name = std::string(method_name);
            DEBUGLOG("parse method_name=%s", message->m_method_name.c_str());

            int err_code_index = method_name_index + message->m_method_name_len; 
            if(err_code_index >= end_index) {
                message->parse_success = false;
                ERRORLOG("parse error, err_code_index[%d] >= end_index[%d]", err_code_index, end_index);
                continue;
            }
            message->m_err_code = getInt32FromNetByte(&tmp[err_code_index]);

            int error_info_len_index = err_code_index + sizeof(message->m_err_code);
            if(error_info_len_index >= end_index) {
                message->parse_success = false;
                ERRORLOG("parse error, error_info_len_index[%d] >= end_index[%d]", error_info_len_index, end_index);
                continue;
            }
            message->m_err_info_len = getInt32FromNetByte(&tmp[error_info_len_index]);

            int err_info_index = error_info_len_index + sizeof(message->m_err_info_len);
            char error_info[512] = {0};
            memcpy(&error_info[0], &tmp[err_info_index], message->m_err_info_len);
            message->m_err_info = std::string(error_info);
            DEBUGLOG("parse err_info=%s", message->m_err_info.c_str());

            int pb_data_len = message->m_pk_len - message->m_method_name_len - message->m_msg_id_len - message->m_err_info_len
                                - 2 - 6 * 4; // PB_BEGIN PB_END 6*int32_t
            int pb_data_index = err_info_index + message->m_err_info_len;
            message->m_pb_data = std::string(&tmp[pb_data_index], pb_data_len);

            // check sum

            message->parse_success = true;

            out_messages.push_back(message);
            
        }

    }



}

const char* TinyPBCoder::encodeTinyPB(std::shared_ptr<TinyPBProtocal> message, int& len) {
    if(message->m_msg_id.empty()) {
        message->m_msg_id = "123456789";
    }
    DEBUGLOG("msg_id=%s", message->m_msg_id.c_str());
    int pk_len = 2 + 24 +  message->m_method_name.length() + message->m_msg_id.length() + message->m_err_info.length() + message->m_pb_data.length();
    DEBUGLOG("pk_len = %d", pk_len);

    char* buf = reinterpret_cast<char*>(malloc(pk_len));
    char* tmp = buf;
    *tmp = TinyPBProtocal::PB_START;
    tmp++;

    int32_t pk_len_net = htonl(pk_len);
    memcpy(tmp, &pk_len_net, sizeof(pk_len_net));
    tmp += sizeof(pk_len_net);

    int msg_id_len = message->m_msg_id.length();
    int32_t msg_id_len_net = htonl(msg_id_len);
    memcpy(tmp, &msg_id_len_net, sizeof(msg_id_len_net));
    tmp += sizeof(msg_id_len_net);

    if(!message->m_msg_id.empty()) {
        memcpy(tmp, &(message->m_msg_id[0]), msg_id_len);
        tmp += msg_id_len;
    }

    int method_name_len = message->m_method_name.length();
    int32_t method_name_len_net = htonl(method_name_len);
    memcpy(tmp, &method_name_len_net, sizeof(method_name_len_net));
    tmp += sizeof(method_name_len_net);

    if(!message->m_method_name.empty()) {
        memcpy(tmp, &(message->m_method_name[0]), method_name_len);
        tmp += method_name_len;
    }

    int32_t err_code_net = htonl(message->m_err_code);
    memcpy(tmp, &err_code_net, sizeof(err_code_net));
    tmp += sizeof(err_code_net);

    int err_info_len = message->m_err_info.length();
    int32_t err_info_len_net = htonl(err_info_len);
    memcpy(tmp, &err_info_len_net, sizeof(err_info_len_net));
    tmp += sizeof(err_info_len_net);

    if(!message->m_err_info.empty()) {
        memcpy(tmp, &(message->m_err_info[0]), err_info_len);
        tmp += err_info_len;
    }

    if(!message->m_pb_data.empty()) {
        memcpy(tmp, &(message->m_pb_data[0]), message->m_pb_data.length());
        tmp += message->m_pb_data.length();
    }

    int32_t check_sum_net = htonl(1);
    memcpy(tmp, &check_sum_net, sizeof(check_sum_net));
    tmp += sizeof(check_sum_net);

    *tmp = TinyPBProtocal::PB_END;

    message->m_pk_len = pk_len;
    message->m_msg_id_len = msg_id_len;
    message->m_method_name_len = method_name_len;
    message->m_err_info_len = err_info_len;
    message->parse_success = true;
    len = pk_len;

    DEBUGLOG("encode message[%s] success", message->m_msg_id.c_str());

    return buf;
}
    
} // namespace myRPC



