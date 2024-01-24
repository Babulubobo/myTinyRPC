#ifndef MYRPC_COMMON_LOG_H
#define MYRPC_COMMON_LOG_H

#include <string>

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

enum LogLevel {
    Debug = 1,
    Info = 2,
    Error = 3
};

class LogEvent {
public:
    std::string getFileName() const {
        return m_file_name;
    }
    LogLevel getLogLevel() const {
        return m_level;
    }
    void printLog();
    
private:
    std::string m_file_name; // file name
    int32_t m_file_line; // line number
    int32_t m_pid; // pid number
    int32_t m_thread_id; // thread id

    LogLevel m_level; // level of log
    
};

}

#endif