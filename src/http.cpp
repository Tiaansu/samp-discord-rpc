#include <vector>
#include "http.hpp"

namespace HTTP
{
    WebRequest::WebRequest(const std::string& agent, const std::string& domain, const INTERNET_PORT port, DWORD timeout)
    {
        internet = InternetOpenA(agent.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
        if (!internet)
        {
            throw std::runtime_error("InternetOpen failed");
        }

        InternetSetOptionA(internet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
        InternetSetOptionA(internet, INTERNET_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
        InternetSetOptionA(internet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));

        session = InternetConnectA(internet, domain.c_str(), port, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
        if (!session)
        {
            InternetCloseHandle(internet);
            throw std::runtime_error("InternetConnect failed");
        }
    }

    WebRequest::~WebRequest()
    {
        InternetCloseHandle(internet);
        InternetCloseHandle(session);
    }

    std::pair<std::string, DWORD> WebRequest::ReadRequest(HINTERNET request)
    {
        std::string headers;
        DWORD httpStatus = 0;

        DWORD statusCodeSize = sizeof(httpStatus);
        if (!HttpQueryInfoA(request, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &httpStatus, &statusCodeSize, NULL))
        {
            return std::make_pair("", 0);
        }

        std::string response;
        std::vector<char> buffer(4096);
        DWORD bytesRead;
        while (InternetReadFile(request, buffer.data(), buffer.size(), &bytesRead) && bytesRead > 0)
        {
            response.append(buffer.data(), bytesRead);
        }

        return std::make_pair(response, httpStatus);
    }

    HINTERNET WebRequest::SendRequest(const char* location, const char* method, const std::string& data)
    {
        LPCSTR accept[] = {"application/json", nullptr};
        DWORD flags = INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RELOAD;
        if (session)
        {
            flags |= INTERNET_FLAG_SECURE;
        }

        HINTERNET request = HttpOpenRequestA(session, method, location, nullptr, nullptr, accept, flags, 0);
        if (!request)
        {
            return nullptr;
        }

        std::string contentType = "Content-Type: application/json";
        if (!HttpAddRequestHeadersA(request, contentType.c_str(), (DWORD)contentType.length(), HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE))
        {
            InternetCloseHandle(request);
            return nullptr;
        }

        if (!HttpSendRequestA(request, NULL, 0, (LPVOID)data.c_str(), (DWORD)data.length()))
        {
            InternetCloseHandle(request);
            return nullptr;
        }

        return request;
    }

    nlohmann::json WebRequest::Post(const char* location, const nlohmann::json& jsonData)
    {
        std::string data = jsonData.dump();
        HINTERNET request = SendRequest(location, "POST", data);

        if (!request)
        {
            return nlohmann::json();
        }

        auto [response, httpStatus] = ReadRequest(request);
        InternetCloseHandle(request);

        if (httpStatus != HTTP_STATUS_OK)
        {
            return nlohmann::json();
        }
        if (response.empty())
        {
            return nlohmann::json();
        }
        
        try
        {
            return nlohmann::json::parse(response, nullptr, true /* allow_exceptions */, true /* ignore_comments */);
        }
        catch (const nlohmann::json::parse_error& e)
        {
            return nlohmann::json();
        }
    }

    nlohmann::json WebRequest::Get(const char* location)
    {
        HINTERNET request = SendRequest(location, "GET");
        if (!request)
        {
            return nlohmann::json();
        }

        auto [response, httpStatus] = ReadRequest(request);
        InternetCloseHandle(request);

        if (httpStatus != HTTP_STATUS_OK)
        {
            return nlohmann::json();
        }
        if (response.empty())
        {
            return nlohmann::json();
        }

        try
        {
            return nlohmann::json::parse(response, nullptr, true /* allow_exceptions */, true /* ignore_comments */);
        }
        catch (const nlohmann::json::parse_error& e)
        {
            return nlohmann::json();
        }
    }
}