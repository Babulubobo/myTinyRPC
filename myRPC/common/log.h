#ifndef MYRPC_COMMON_LOG_H
#define MYRPC_COMMON_LOG_H

#include <queue>
#include <string>
#include <memory>
#include <semaphore.h>

#include "myRPC/common/config.h"
#include "myRPC/common/mutex.h"
#include "myRPC/net/timer_event.h"

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
    myRPC::Logger::GetGlobalLogger()->pushLog(myRPC::LogEvent(myRPC::LogLevel::Debug).toString() \
     + "[" + std::string(__FILE__) + ":" +std::to_string(__LINE__ ) + "]\t" + myRPC::formatString(str, ##__VA_ARGS__) + '\n');\
    }\
    
#define INFOLOG(str, ...)\
    if(myRPC::Logger::GetGlobalLogger()->getLogLevel() <= myRPC::Info) { \
    myRPC::Logger::GetGlobalLogger()->pushLog(myRPC::LogEvent(myRPC::LogLevel::Info).toString() \
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__ ) + "]\t" + myRPC::formatString(str, ##__VA_ARGS__) + '\n');\
    }\

#define ERRORLOG(str, ...)\
    if(myRPC::Logger::GetGlobalLogger()->getLogLevel() <= myRPC::Error) { \
    myRPC::Logger::GetGlobalLogger()->pushLog(myRPC::LogEvent(myRPC::LogLevel::Error).toString() \
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__ ) + "]\t" + myRPC::formatString(str, ##__VA_ARGS__) + '\n');\
    }\



#define APPDEBUGLOG(str, ...)\
    if(myRPC::Logger::GetGlobalLogger()->getLogLevel() <= myRPC::Debug) { \
    myRPC::Logger::GetGlobalLogger()->pushAppLog(myRPC::LogEvent(myRPC::LogLevel::Debug).toString() \
     + "[" + std::string(__FILE__) + ":" +std::to_string(__LINE__ ) + "]\t" + myRPC::formatString(str, ##__VA_ARGS__) + '\n');\
    }\
    
#define APPINFOLOG(str, ...)\
    if(myRPC::Logger::GetGlobalLogger()->getLogLevel() <= myRPC::Info) { \
    myRPC::Logger::GetGlobalLogger()->pushAppLog(myRPC::LogEvent(myRPC::LogLevel::Info).toString() \
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__ ) + "]\t" + myRPC::formatString(str, ##__VA_ARGS__) + '\n');\
    }\

#define APPERRORLOG(str, ...)\
    if(myRPC::Logger::GetGlobalLogger()->getLogLevel() <= myRPC::Error) { \
    myRPC::Logger::GetGlobalLogger()->pushAppLog(myRPC::LogEvent(myRPC::LogLevel::Error).toString() \
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__ ) + "]\t" + myRPC::formatString(str, ##__VA_ARGS__) + '\n');\
    }\



enum LogLevel {
    Unknown = 0,
    Debug = 1,
    Info = 2,
    Error = 3
};



class AsyncLogger {

public:
    typedef std::shared_ptr<AsyncLogger> s_ptr;

    AsyncLogger(const std::string& file_name, const std::string& file_path, int max_size);

    void stop();

    // 刷新到磁盘
    void flush();

    void pushLogBuffer(std::vector<std::string>& vec);

public:
    static void* Loop(void* arg);

private:

    std::queue<std::vector<std::string>> m_buffer;
    
    // m_file_path/m_file_name_yyyymmdd.0
    std::string m_file_name; // log output file name
    std::string m_file_path; // log output file path

    int m_max_file_size {0}; // max single log file size, measured in bytes

    sem_t m_semaphore;

    pthread_t m_thread;

    pthread_cond_t m_condition; // 条件变量???
    Mutex m_mutex;

    std::string m_date; //当前打印的日志文件日期
    FILE* m_file_handler {nullptr}; //当前打开的日志文件句柄

    bool m_reopen_flag {false};

    int m_no {0}; //日志文件序号

    bool m_stop_flag {false};

};



class Logger {
public:
    Logger(LogLevel level, int type = 1);

    typedef std::shared_ptr<Logger> s_ptr;

    LogLevel getLogLevel() const {
        return m_set_level;
    }

    void pushLog(const std::string& msg);

    void pushAppLog(const std::string& msg);

    void log();

    void syncLoop();

    void init();

public:
    static Logger* GetGlobalLogger();
    static void InitGlobalLogger(int type = 1);
    
private:
    LogLevel m_set_level;

    std::vector<std::string> m_buffer;
    std::vector<std::string> m_app_buffer;

    Mutex m_mutex;

    Mutex m_app_mutex;

    // m_file_path/m_file_name_yyyymmdd.1
    std::string m_file_name; // log output file name
    std::string m_file_path; // log output file path

    int m_max_file_size {0}; // max single log file size

    AsyncLogger::s_ptr m_async_logger;

    AsyncLogger::s_ptr m_async_app_logger;

    TimerEvent::s_ptr m_timer_event;

    int m_type {0};  // 1: async log, 0: sync log

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