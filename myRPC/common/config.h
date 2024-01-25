#ifndef MYRPC_COMMON_CONFIG_H
#define MYRPC_COMMON_CONFIG_H

#include <map>
#include <string>

namespace myRPC {

class Config {
public:
    Config(const char* xmlfile);

public:
    static Config* GetGlobalConfig();
    static void SetGlobalConfig(const char* xmlfile);

public:
    // std::map<std::string, std::string> m_config_values;
    std::string m_log_level;
};

}

#endif