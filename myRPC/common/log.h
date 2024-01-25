#ifndef MYRPC_COMMON_LOG_H
#define MYRPC_COMMON_LOG_H

#include <queue>
#include <string>
#include <memory>

#include "myRPC/common/config.h"
#include "myRPC/common/mutex.h"

namespace myRPC {

template<typename... Args>
std::string formatString(const char* str, Args&&... args) {
    int size = snprintf(nullptr, 0, str, args...); // nullptr and 0: snprintf return the size needed for the formatted(args) string

    std::string result;

    //check if snprintf success
    if(size > 0) {
        result.resize(size);
        // Use snprintf to write the formatted string into the result string
        // Write "size" characters and one '\0' into the string.
        // std::string automatically appends a null character '\0' at the end, so the result string size is "size"
        snprintf(&result[0], size+1, str, args...);
    }

    return result;
}



#define DEBUGLOG(str, ...)\
    if(myRPC::Logger::GetGlobalLogger()->getLogLevel() <= myRPC::Debug) { \
    myRPC::Logger::GetGlobalLogger()->pushLog((new myRPC::LogEvent(myRPC::LogLevel::Debug))->toString() \
     + "[" + std::string(__FILE__) + ":" +std::to_string(__LINE__ ) + "]\t" + myRPC::formatString(str, ##__VA_ARGS__) + '\n');\
    myRPC::Logger::GetGlobalLogger()->log();\
    }\
    
#define INFOLOG(str, ...)\
    if(myRPC::Logger::GetGlobalLogger()->getLogLevel() <= myRPC::Info) { \
    myRPC::Logger::GetGlobalLogger()->pushLog((new myRPC::LogEvent(myRPC::LogLevel::Info))->toString() \
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__ ) + "]\t" + myRPC::formatString(str, ##__VA_ARGS__) + '\n');\
    myRPC::Logger::GetGlobalLogger()->log();\
    }\

#define ERRORLOG(str, ...)\
    if(myRPC::Logger::GetGlobalLogger()->getLogLevel() <= myRPC::Error) { \
    myRPC::Logger::GetGlobalLogger()->pushLog((new myRPC::LogEvent(myRPC::LogLevel::Error))->toString() \
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__ ) + "]\t" + myRPC::formatString(str, ##__VA_ARGS__) + '\n');\
    myRPC::Logger::GetGlobalLogger()->log();\
    }\



enum LogLevel {
    Unknown = 0,
    Debug = 1,
    Info = 2,
    Error = 3
};



class Logger {
public:
    Logger(LogLevel level) : m_set_level(level) {};

    typedef std::shared_ptr<Logger> s_ptr;

    LogLevel getLogLevel() const {
        return m_set_level;
    }

    void pushLog(const std::string& msg);

    void log();

public:
    static Logger* GetGlobalLogger();
    static void InitGlobalLogger();
    
private:
    LogLevel m_set_level;

    std::queue<std::string> m_buffer;

    Mutex m_mutex;
};



std::string LogLevelToString(LogLevel level);

LogLevel StringToLogLevel(const std::string& log_level);



class LogEvent {
public:
    LogEvent(LogLevel level) : m_level(level) {};

    std::string getFileName() const {
        return m_file_name;
    }
    LogLevel getLogLevel() const {
        return m_level;
    }
    std::string toString();

private:
    std::string m_file_name; // file name
    int32_t m_file_line; // line number
    int32_t m_pid; // pid number
    int32_t m_thread_id; // thread id

    LogLevel m_level; // level of log
    
};



}

#endif