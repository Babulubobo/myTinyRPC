#include <sys/time.h>
#include <time.h>
#include "myRPC/common/log.h"

namespace myRPC{
    void LogEvent::printLog() {
        // In C, declaring a structure variable must have the keyword "struct"
        struct timeval now_time;
        gettimeofday(&now_time, nullptr);

        struct tm now_time_t;
        localtime_r(&(now_time.tv_sec), &now_time_t); // MT-safe

        char buf[128];
        strftime(&buf[0], 128, "%y-%m-%d %H:%M:%S", &now_time_t);
        std::string time_str(buf); // initialize string using char[]
        int ms = now_time.tv_usec * 1000;
        time_str += '.' + std::to_string(ms);
    }
}