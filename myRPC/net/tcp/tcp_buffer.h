#ifndef MYRPC_NET_TCP_TCP_BUFFER_H
#define MYRPC_NET_TCP_TCP_BUFFER_H

#include <vector>
#include <memory>

namespace myRPC
{
    
class TcpBuffer {

public:

    typedef std::shared_ptr<TcpBuffer> s_ptr;

    TcpBuffer(int size);

    ~TcpBuffer();

    // return byte number that can read
    int readAble();

    // return byte number that can write
    int writeAble();

    int readIndex();

    int writeIndex();

    void writeToBuffer(const char* buf, int size);

    void readFromBuffer(std::vector<char>& re, int size);

    void resizeBuffer(int new_size);

    void adjustBuffer();

    void moveReadIndex(int size); // only add

    void moveWriteIndex(int size);// only add

private:
    int m_read_index {0};

    int m_write_index {0};

    int m_size {0};
public:
    std::vector<char> m_buffer;

};


} // namespace myRPC


#endif