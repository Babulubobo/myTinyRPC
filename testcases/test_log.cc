#include <pthread.h>
#include <iostream>
#include "myRPC/common/log.h"
#include "myRPC/common/config.h"

void* func(void*) {
    int i = 20;
    while(i--) {
        DEBUGLOG("debug this is thread in %s", "func");
        INFOLOG("info this is thread in %s", "func");
    }
    
    return NULL;
}

int main() {

    myRPC::Config::SetGlobalConfig("../conf/myRPC.xml");
    myRPC::Logger::InitGlobalLogger();

    pthread_t thd;
    pthread_create(&thd, NULL, &func, NULL);
    
    int i = 20;
    while(i--) {
        DEBUGLOG("test debug log %s", "func");
        INFOLOG("info debug log %s", "func");
    }
    pthread_join(thd, NULL);
    return 0;
}