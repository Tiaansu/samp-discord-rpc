#pragma once

#include <string>

namespace SAMP
{
    enum ConnectMode
    {
        SAMP_CONNECT_UNKNOWN,
        SAMP_CONNECT_SERVER,
        SAMP_CONNECT_DEBUG
    };

    struct ServerInfo
    {
        ConnectMode connect = SAMP_CONNECT_UNKNOWN;
        std::string address;
        std::string port;
        std::string username;
    };

    bool ReadServerInfo(const char* cmdline, ServerInfo& data);
}