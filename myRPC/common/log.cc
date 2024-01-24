#include <sstream>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include "myRPC/common/log.h"
#include "myRPC/common/util.h"

#define DEBUGLOG(str, ...)\
    string msg = (new myRPC::LogEvent(myRPC::LogLevel::Debug)->toString()) + myRPC::formatString(str, ##__VA_ARGS__);\
    myRPC::g_logger->pushLog(msg);\
    myRPC::g_logger->log();\

namespace myRPC{

static Logger* g_logger = nullptr;

Logger* Logger::GetGlobalLogger() {
    if(g_logger != nullptr) {
        return g_logger;
    }
    return new Logger();
}

std::string LogLevelToString(LogLevel level){
    switch(level) {
    case Debug:
        return "DEBUG";
    case Info:
        return "INFO";
    case Error:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

std::string LogEvent::toString() {
    // In C, declaring a structure variable must have the keyword "struct"
    struct timeval now_time;
    gettimeofday(&now_time, nullptr);

    struct tm now_time_t;
    localtime_r(&(now_time.tv_sec), &now_time_t); // MT-safe

    char buf[128];
    strftime(&buf[0], 128, "%y-%m-%d %H:%M:%S", &now_time_t);
    std::string time_str(buf); // initialize string using char[]
    int ms = now_time.tv_usec / 1000;
    time_str += '.' + std::to_string(ms);

    m_pid = getPid();
    m_thread_id = getThreadId();


    std::stringstream ss;

    ss << "[" << LogLevelToString(m_level) << "]\t"
        << "[" << time_str << "]\t"
        << "[" << std::string(__FILE__) << __LINE__ << "]\t";

    return ss.str();

}

void Logger::pushLog(const std::string& msg){
    m_buffer.push(msg);
}

void Logger::log() {
    while(!m_buffer.empty()){
        std::string msg = m_buffer.front();
        m_buffer.pop();

        printf(msg.c_str());
    }
}

}