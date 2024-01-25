#include <pthread.h>
#include <iostream>
#include "myRPC/common/log.h"

void* func(void*) {
    DEBUGLOG("this is thread in %s", "fun");
    return NULL;
}

int main() {

    pthread_t thd;
    pthread_create(&thd, NULL, &func, NULL);
    pthread_join(thd, NULL);
    DEBUGLOG("test log %s", "11");
    return 0;
}