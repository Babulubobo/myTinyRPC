#ifndef MYRPC_COMMON_MSG_ID_UTIL_H
#define MYRPC_COMMON_MSG_ID_UTIL_H

#include <string>

namespace myRPC
{

class MsgIDUtil {
public:
    static std::string GenMsgID();
};   
    
} // namespace myRPC


#endif