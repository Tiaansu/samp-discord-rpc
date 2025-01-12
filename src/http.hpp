#pragma once

#include <Windows.h>
#include <Wininet.h>
#include <string>
#include "json.hpp"

#pragma comment(lib, "Wininet.lib")

namespace HTTP 
{
    class WebRequest 
    {
    public:
        WebRequest(const std::string& agent, const std::string& domain, const INTERNET_PORT port = INTERNET_DEFAULT_HTTP_PORT, DWORD timeout = 1000);
        ~WebRequest();

        nlohmann::json Post(const char* location, const nlohmann::json& jsonData);
        nlohmann::json Get(const char* location);

    private:
        std::pair<std::string, DWORD> ReadRequest(HINTERNET request);
        HINTERNET SendRequest(const char* location, const char* method, const std::string& data = "");

        HINTERNET internet;
        HINTERNET session;
    };
}