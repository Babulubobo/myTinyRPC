#include "myRPC/common/log.h"
#include "myRPC/net/tcp/net_addr.h"

int main() {

    myRPC::Config::SetGlobalConfig("../conf/myRPC.xml");
    myRPC::Logger::InitGlobalLogger();

    myRPC::IPNetAddr addr("127.0.0.1", 12345);
    DEBUGLOG("create addr %s", addr.toString().c_str());
    return 0;
}