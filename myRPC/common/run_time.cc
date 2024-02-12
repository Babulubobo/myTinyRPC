#include "myRPC/common/run_time.h"

namespace myRPC
{

thread_local Runtime* t_run_time = nullptr;

Runtime* Runtime::GetRunTime() {
    if(t_run_time) {
        return t_run_time;
    }
    t_run_time = new Runtime();
    return t_run_time;
}
    
} // namespace myRPC
