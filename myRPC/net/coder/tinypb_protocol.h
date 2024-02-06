#ifndef MYRPC_NET_CODER_TINYPB_PROTOCOL_H
#define MYRPC_NET_CODER_TINYPB_PROTOCOL_H

#include <string>
#include "myRPC/net/coder/abstract_protocol.h"

namespace myRPC
{

struct TinyPBProtocal : AbstractProtocol {

public:
    static char PB_START;
    static char PB_END;

public:
    int32_t m_pk_len {0}; // all length include PB_START PB_END

    // req_id is in AbstractProtocol
    int32_t m_req_id_len {0};

    int32_t m_method_name_len {0};
    std::string m_method_name;

    int32_t m_err_code {0};
    int32_t m_err_info_len {0};
    std::string m_err_info;

    std::string m_pb_data;

    int32_t m_check_sum {0};

    bool parse_success {false};

};

char TinyPBProtocal::PB_START = 0x02;
char TinyPBProtocal::PB_END = 0x03;


} // namespace myRPC


#endif