#include <memory>
#include <WS2tcpip.h>
#include <iostream>

#include "client.hpp"

namespace SAMP
{
    bool ReadServerInfo(const char* cmdline, ServerInfo& data)
    {
        auto input = std::unique_ptr<char>(_strdup(cmdline));
        char* next = nullptr;
        char* token = strtok_s(input.get(), " ", &next);
        while (token != nullptr)
        {
            if (!std::strcmp(token, "-c"))
            {
                if (data.connect != SAMP_CONNECT_UNKNOWN)
                {
                    return false;
                }
                data.connect = SAMP_CONNECT_SERVER;
            }
            else if (!std::strcmp(token, "-d"))
            {
                if (data.connect != SAMP_CONNECT_UNKNOWN)
                {
                    return false;
                }
                data.connect= SAMP_CONNECT_DEBUG;
            }
            else if (!std::strcmp(token, "-h"))
            {
                if (!next)
                {
                    return false;
                }
                data.address = strtok_s(nullptr, " ", &next);
            }
            else if (!std::strcmp(token, "-p"))
            {
                if (!next)
                {
                    return false;
                }
                data.port = strtok_s(nullptr, " ", &next);
            }
            else if (!std::strcmp(token, "-n"))
            {
                if (!next)
                {
                    return false;
                }
                data.username = strtok_s(nullptr, " ", &next);
            }
            token = strtok_s(nullptr, " ", &next);
        }
        return true;
    }
}