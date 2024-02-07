#ifndef MYRPC_COMMON_ERROR_CODE_H
#define MYRPC_COMMON_ERROR_CODE_H

#ifndef SYS_ERROR_PREFIX //system error prefix
#define SYS_ERROR_PREFIX(xx) 1000##xx
#endif

const int ERROR_PEER_CLOSE = SYS_ERROR_PREFIX(0000); // Peer closed the connection during connection establishment.
const int ERROR_FAILED_CONNECT = SYS_ERROR_PREFIX(0001); // Connect failed
const int ERROR_FAILED_GET_REPLY = SYS_ERROR_PREFIX(0002); // Failed to get peer reply
const int ERROR_FAILED_DESERIALIZE = SYS_ERROR_PREFIX(0003);    // Deserialize failed
const int ERROR_FAILED_SERIALIZE = SYS_ERROR_PREFIX(0004);      // Serialize failed

const int ERROR_FAILED_ENCODE = SYS_ERROR_PREFIX(0005);      // Encode failed
const int ERROR_FAILED_DECODE = SYS_ERROR_PREFIX(0006);      // Decode failed

const int ERROR_RPC_CALL_TIMEOUT = SYS_ERROR_PREFIX(0007);    // Rpc call time out
const int ERROR_SERVICE_NOT_FOUND = SYS_ERROR_PREFIX(0008);    // Service not found
const int ERROR_METHOD_NOT_FOUND = SYS_ERROR_PREFIX(0009);    // Method not found 
const int ERROR_PARSE_SERVICE_NAME = SYS_ERROR_PREFIX(0010);    // Service name parse failed
// const int ERROR_RPC_CHANNEL_INIT = SYS_ERROR_PREFIX(0011);    // Rpc channel init failed
// const int ERROR_RPC_PEER_ADDR = SYS_ERROR_PREFIX(0012);    // Rpc peer addr error


#endif