#ifndef MYRPC_NET_ABSTRACT_PROTOCOL_H
#define MYRPC_NET_ABSTRACT_PROTOCOL_H

#include <memory>

namespace myRPC
{

class AbstractProtocol {

public:
    typedef std::shared_ptr<AbstractProtocol> s_ptr;


};
    
} // namespace myRPC


#endif