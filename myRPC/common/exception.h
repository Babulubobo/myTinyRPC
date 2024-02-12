#ifndef MYRPC_COMMON_EXCEPTION_H
#define MYRPC_COMMON_EXCEPTION_H

#include <exception>
#include <string>

namespace myRPC {

class MyRPCException : public std::exception {
 public:

  MyRPCException(int error_code, const std::string& error_info) : m_error_code(error_code), m_error_info(error_info) {}

  // 异常处理
  // 当捕获到 MyRPCException 及其子类对象的异常时，会执行该函数
  virtual void handle() = 0;

  virtual ~MyRPCException() {};

  int errorCode() {
    return m_error_code;
  }

  std::string errorInfo() {
    return m_error_info;
  }

 protected:
  int m_error_code {0};

  std::string m_error_info;

};


}



#endif