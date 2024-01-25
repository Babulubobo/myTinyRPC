#include <sstream>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include "myRPC/common/log.h"
#include "myRPC/common/util.h"
#include "myRPC/common/config.h"


namespace myRPC{

static Logger* g_logger = nullptr;

Logger* Logger::GetGlobalLogger() {
    return g_logger;
}

void Logger::InitGlobalLogger(){
    LogLevel global_log_level = StringToLogLevel(Config::GetGlobalConfig()->m_log_level);
    printf("Init log level [%s]\n", LogLevelToString(global_log_level).c_str());
    g_logger = new Logger(global_log_level);
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

LogLevel StringToLogLevel(const std::string& log_level) {
    if(log_level == "DEBUG") return Debug;
    else if(log_level == "INFO") return Info;
    else if(log_level == "ERROR") return Error;
    return Unknown;
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
        << "[" << m_pid << ":" << m_thread_id << "]\t";

    return ss.str();

}

void Logger::pushLog(const std::string& msg){
    ScopeMutex<Mutex> logLock(m_mutex);
    m_buffer.push(msg);
    logLock.unlock();
}

void Logger::log() {

    ScopeMutex<Mutex> logLock(m_mutex);
    std::queue<std::string> tmp = m_buffer;
    m_buffer.swap(tmp);

    logLock.unlock();

    while(!tmp.empty()){
        std::string msg = tmp.front();
        tmp.pop();

        printf(msg.c_str());
    }
}

}