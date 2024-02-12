#ifndef MYRPC_COMMON_RUN_TIME_H
#define MYRPC_COMMON_RUN_TIME_H

#include <string>

namespace myRPC
{

class Runtime {

public:

public:
    static Runtime* GetRunTime();

public:
    std::string m_msgid;

    std::string m_method_name;


};


} // namespace myRPC


#endif