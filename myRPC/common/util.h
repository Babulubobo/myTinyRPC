#ifndef MYRPC_COMMON_UTIL_H
#define MYRPC_COMMON_UTIL_H

#include <sys/types.h>
#include <unistd.h>

namespace myRPC {
    pid_t getPid();

    pid_t getThreadId();

    int64_t getNowMs();

    int32_t getInt32FromNetByte(const char* buf);
}

#endif