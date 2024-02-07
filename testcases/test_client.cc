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
#include "myRPC/net/tcp/tcp_client.h"

#include "myRPC/net/tcp/net_addr.h"
#include "myRPC/net/tcp/tcp_server.h"
#include "myRPC/net/coder/string_coder.h"
#include "myRPC/net/coder/abstract_protocol.h"
#include "myRPC/net/coder/tinypb_coder.h"
#include "myRPC/net/coder/tinypb_protocol.h"

#include <iostream>

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

    rt = read(fd, buf, 100); // read func don't add '\0'!!!
    buf[rt] = '\0';

    DEBUGLOG("success read %d bytes, [%s]", std::string(buf).size(), std::string(buf).c_str());
}

void test_tcp_client() {
    myRPC::IPNetAddr::s_ptr addr = std::make_shared<myRPC::IPNetAddr>("127.0.0.1", 12345);
    myRPC::TcpClient client(addr);
    client.connect([addr, &client]() { // &client???
        DEBUGLOG("connect to [%s] success", addr->toString().c_str());
        std::shared_ptr<myRPC::TinyPBProtocal> message = std::make_shared<myRPC::TinyPBProtocal>();
        message->m_req_id = "123456789";
        message->m_pb_data = "test pb data";

        client.writeMessage(message, [](myRPC::AbstractProtocol::s_ptr done) {
            DEBUGLOG("send message success");
        });
        client.readMessage("123456789", [](myRPC::AbstractProtocol::s_ptr done) {
            // ???: dynamic_pointer_cast: used to change base class ptr to derived class ptr (both are shared ptr)
            std::shared_ptr<myRPC::TinyPBProtocal> message = std::dynamic_pointer_cast<myRPC::TinyPBProtocal>(done);
            DEBUGLOG("req_Id [%s], get response %s", message->m_req_id.c_str(), message->m_pb_data.c_str());
        });

    });
}


int main() {

    myRPC::Config::SetGlobalConfig("../conf/myRPC.xml");
    myRPC::Logger::InitGlobalLogger();

    // test_connect();

    test_tcp_client();

    return 0;
}