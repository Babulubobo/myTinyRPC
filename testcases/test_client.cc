#include <assert.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>
#include <unistd.h>
#include "myRPC/common/log.h"
#include "myRPC/common/config.h"

void test_connect() {
    // call "connect" function to connect server
    // write a string
    // wait "read" return result
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if(fd < 0) {
        ERRORLOG("invalid fd %d", fd);
        exit(0);
    }
    
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    inet_aton("127.0.0.1", &server_addr.sin_addr);

    int rt = connect(fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));

    DEBUGLOG("connect success");

    std::string msg = "hello my rpc!";

    rt = write(fd, msg.c_str(), msg.length());
    // std::cout << rt << std::endl;

    DEBUGLOG("success write %d bytes, [%s]", rt, msg.c_str());

    char buf[100];
    rt = read(fd, buf, 100);

    DEBUGLOG("success read %d bytes, [%s]", std::string(buf).size(), std::string(buf).c_str());
}


int main() {

    myRPC::Config::SetGlobalConfig("../conf/myRPC.xml");
    myRPC::Logger::InitGlobalLogger();

    test_connect();

    return 0;
}