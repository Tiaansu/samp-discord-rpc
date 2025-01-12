// taken from: https://github.com/openmultiplayer/launcher/blob/master/src-tauri/src/query.rs
// converted by gemini ai

#include "query.hpp"
#include <windows.h>
#include <regex>
#include <codecvt>
#include <locale>

#pragma comment(lib, "ws2_32.lib")

std::string ws2s(const std::wstring& wstr) 
{
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::wstring s2ws(const std::string& str) 
{
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

bool initialize_winsock() 
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) 
    {
        return false;
    }
    return true;
}

void cleanup_winsock() 
{
    WSACleanup();
}

Query::Query() : address(""), port(0), sockfd(INVALID_SOCKET) 
{
    initialize_winsock();
}

Query::~Query() 
{
    if (sockfd != INVALID_SOCKET) 
    {
        closesocket(sockfd);
    }
    cleanup_winsock();
}

bool Query::new_query(const std::string& addr, int port) 
{
    this->port = port;
    std::regex ipv4_regex(R"(^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)");
    std::smatch match;

    if (std::regex_match(addr, match, ipv4_regex)) 
    {
        this->address = addr;
    }
    else 
    {
        addrinfo hints{}, * result;
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;

        if (getaddrinfo(addr.c_str(), std::to_string(port).c_str(), &hints, &result) != 0) 
        {
            return false;
        }

        for (addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) 
        {
            if (ptr->ai_family == AF_INET) 
            {
                sockaddr_in* sockaddr_ipv4 = (sockaddr_in*)ptr->ai_addr;
                char ipstr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(sockaddr_ipv4->sin_addr), ipstr, INET_ADDRSTRLEN);
                this->address = ipstr;
                break;
            }
        }
        freeaddrinfo(result);
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) 
    {
        return false;
    }

    DWORD timeout = 2000;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0) 
    {
        closesocket(sockfd);
        return false;
    }

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, this->address.c_str(), &serv_addr.sin_addr);

    if (connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
    {
        closesocket(sockfd);
        return false;
    }

    return true;
}

int Query::send_query(char query_type) 
{
    std::vector<unsigned char> packet;
    std::string samp = "SAMP";
    packet.insert(packet.end(), samp.begin(), samp.end());

    unsigned char ip_bytes[4];
    inet_pton(AF_INET, address.c_str(), &ip_bytes);
    packet.insert(packet.end(), ip_bytes, ip_bytes + 4);

    packet.push_back(port & 0xFF);
    packet.push_back((port >> 8) & 0xFF);
    packet.push_back(static_cast<unsigned char>(query_type));

    if (query_type == 'p') 
    {
        packet.push_back(0);
        packet.push_back(0);
        packet.push_back(0);
        packet.push_back(0);
    }

    int sent_bytes = send(sockfd, (char*)packet.data(), packet.size(), 0);
    if (sent_bytes == SOCKET_ERROR) 
    {
        return -1;
    }
    return sent_bytes;
}

nlohmann::json Query::recv_query() 
{
    char buf[1500];
    int recv_bytes = recv(sockfd, buf, sizeof(buf), 0);

    if (recv_bytes == SOCKET_ERROR) 
    {
        if (WSAGetLastError() == WSAETIMEDOUT) 
        {
            return {
                {"error", "timeout"}
            };
        }
        return {
            {"error", "error"}
        };
    }

    if (recv_bytes == 0) 
    {
        return {
            {"error", "no data"}
        };
    }

    char query_type = buf[10];
    std::vector<unsigned char> packet(buf + 11, buf + recv_bytes);

    if (query_type == 'i') 
    {
        return build_info_packet(packet);
    }
    else if (query_type == 'c') 
    {
        return build_players_packet(packet);
    }
    else if (query_type == 'r') 
    {
        return build_rules_packet(packet);
    }
    else if (query_type == 'o') 
    {
        return build_extra_info_packet(packet);
    }
    else if (query_type == 'p') 
    {
        return {
            {"ping", "pong"}
        };
    }
    else 
    {
        return {
            "error", "no data"
        };
    }
}

nlohmann::json InfoPacket::to_json() const 
{
    nlohmann::json j;
    j["password"] = password;
    j["players"] = players;
    j["max_players"] = max_players;
    j["hostname"] = hostname;
    j["gamemode"] = gamemode;
    j["language"] = language;
    return j;
}

nlohmann::json Player::to_json() const 
{
    nlohmann::json j;
    j["name"] = name;
    j["score"] = score;
    return j;
}

nlohmann::json ExtraInfoPacket::to_json() const 
{
    nlohmann::json j;
    j["discord_link"] = discord_link;
    j["light_banner_url"] = light_banner_url;
    j["dark_banner_url"] = dark_banner_url;
    j["logo_url"] = logo_url;
    return j;
}

nlohmann::json Query::build_info_packet(std::vector<unsigned char>& packet) 
{
    InfoPacket data;
    size_t pos = 0;

    data.password = packet[pos++] != 0;
    data.players = (packet[pos + 1] << 8) | packet[pos];
    pos += 2;
    data.max_players = (packet[pos + 1] << 8) | packet[pos];
    pos += 2;

    uint32_t hostname_len = (packet[pos + 3] << 24) | (packet[pos + 2] << 16) | (packet[pos + 1] << 8) | packet[pos];
    pos += 4;
    std::vector<unsigned char> hostname_buf(packet.begin() + pos, packet.begin() + pos + hostname_len);
    data.hostname = decode_buffer(hostname_buf).first;
    pos += hostname_len;

    uint32_t gamemode_len = (packet[pos + 3] << 24) | (packet[pos + 2] << 16) | (packet[pos + 1] << 8) | packet[pos];
    pos += 4;
    std::vector<unsigned char> gamemode_buf(packet.begin() + pos, packet.begin() + pos + gamemode_len);
    data.gamemode = decode_buffer(gamemode_buf).first;
    pos += gamemode_len;

    uint32_t language_len = (packet[pos + 3] << 24) | (packet[pos + 2] << 16) | (packet[pos + 1] << 8) | packet[pos];
    pos += 4;
    std::vector<unsigned char> language_buf(packet.begin() + pos, packet.begin() + pos + language_len);
    data.language = decode_buffer(language_buf).first;

    return data.to_json();
}

nlohmann::json Query::build_extra_info_packet(std::vector<unsigned char>& packet) 
{
    ExtraInfoPacket data;
    size_t pos = 0;

    uint32_t discord_link_len = (packet[pos + 3] << 24) | (packet[pos + 2] << 16) | (packet[pos + 1] << 8) | packet[pos];
    pos += 4;
    std::vector<unsigned char> discord_link_buf(packet.begin() + pos, packet.begin() + pos + discord_link_len);
    data.discord_link = decode_buffer(discord_link_buf).first;
    pos += discord_link_len;

    uint32_t banner_url_len = (packet[pos + 3] << 24) | (packet[pos + 2] << 16) | (packet[pos + 1] << 8) | packet[pos];
    pos += 4;
    std::vector<unsigned char> banner_url_buf(packet.begin() + pos, packet.begin() + pos + banner_url_len);
    data.light_banner_url = decode_buffer(banner_url_buf).first;
    pos += banner_url_len;

    banner_url_len = (packet[pos + 3] << 24) | (packet[pos + 2] << 16) | (packet[pos + 1] << 8) | packet[pos];
    pos += 4;
    banner_url_buf.assign(packet.begin() + pos, packet.begin() + pos + banner_url_len);
    data.dark_banner_url = decode_buffer(banner_url_buf).first;
    pos += banner_url_len;

    if (pos < packet.size()) 
    {
        uint32_t logo_url_len = (packet[pos + 3] << 24) | (packet[pos + 2] << 16) | (packet[pos + 1] << 8) | packet[pos];
        pos += 4;
        std::vector<unsigned char> logo_url_buf(packet.begin() + pos, packet.begin() + pos + logo_url_len);
        data.logo_url = decode_buffer(logo_url_buf).first;
    }

    return data.to_json();
}

nlohmann::json Query::build_players_packet(std::vector<unsigned char>& packet) 
{
    size_t pos = 0;
    uint16_t player_count = (packet[pos + 1] << 8) | packet[pos];
    pos += 2;
    std::vector<Player> players(player_count);

    for (int i = 0; i < player_count; ++i) 
    {
        uint8_t player_name_len = packet[pos++];
        std::vector<unsigned char> player_name_buf(packet.begin() + pos, packet.begin() + pos + player_name_len);
        players[i].name = decode_buffer(player_name_buf).first;
        pos += player_name_len;

        players[i].score = (packet[pos + 3] << 24) | (packet[pos + 2] << 16) | (packet[pos + 1] << 8) | packet[pos];
        pos += 4;
    }

    nlohmann::json players_json = nlohmann::json::array();
    for (const auto& player : players) 
    {
        players_json.push_back(player.to_json());
    }

    return players_json;
}

nlohmann::json Query::build_rules_packet(std::vector<unsigned char>& packet) 
{
    size_t pos = 0;
    uint16_t rule_count = (packet[pos + 1] << 8) | packet[pos];
    pos += 2;
    nlohmann::json rules_json = nlohmann::json::object();

    for (int i = 0; i < rule_count; ++i) 
    {
        uint8_t rule_name_len = packet[pos++];
        std::vector<unsigned char> rule_name_buf(packet.begin() + pos, packet.begin() + pos + rule_name_len);
        std::string rule_name = decode_buffer(rule_name_buf).first;
        pos += rule_name_len;

        uint8_t rule_value_len = packet[pos++];
        std::vector<unsigned char> rule_value_buf(packet.begin() + pos, packet.begin() + pos + rule_value_len);
        std::string rule_value = decode_buffer(rule_value_buf).first;
        pos += rule_value_len;

        rules_json[rule_name] = rule_value;
    }

    return rules_json;
}

std::pair<std::string, std::string> Query::decode_buffer(const std::vector<unsigned char>& buf) 
{
    if (buf.empty()) 
    {
        return std::make_pair("", "empty");
    }

    if (buf.size() >= 3 && buf[0] == 0xEF && buf[1] == 0xBB && buf[2] == 0xBF) 
    {
        return std::make_pair(std::string(buf.begin() + 3, buf.end()), "UTF-8");
    }

    if (buf.size() >= 2 && buf[0] == 0xFF && buf[1] == 0xFE) 
    {
        std::wstring wide_string;
        for (size_t i = 2; i + 1 < buf.size(); i += 2) 
        {
            wchar_t wc = (buf[i + 1] << 8) | buf[i];
            wide_string.push_back(wc);
        }
        return std::make_pair(ws2s(wide_string), "UTF-16LE");
    }

    if (buf.size() >= 2 && buf[0] == 0xFE && buf[1] == 0xFF)
    {
        std::wstring wide_string;
        for (size_t i = 2; i + 1 < buf.size(); i += 2) 
        {
            wchar_t wc = (buf[i] << 8) | buf[i + 1];
            wide_string.push_back(wc);
        }
        return std::make_pair(ws2s(wide_string), "UTF-16BE");
    }

    std::wstring wide_string;
    for (size_t i = 0; i < buf.size(); ++i) 
    {
        wide_string.push_back(static_cast<wchar_t>(buf[i]));
    }

    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
    try 
    {
        return std::make_pair(ws2s(wide_string), "Windows-1252");
    }
    catch (const std::range_error& e) 
    {
        return std::make_pair("", "unknown");
    }
}