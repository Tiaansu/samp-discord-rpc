// taken from: https://github.com/openmultiplayer/launcher/blob/master/src-tauri/src/query.rs
// converted by gemini ai

#pragma once

#include <WinSock2.h>
#include <string>
#include <vector>
#include <WS2tcpip.h>
#include "types.hpp"

class Query {
public:
    Query();
    ~Query();

    bool new_query(const std::string& addr, int port);
    int send_query(char query_type);
    nlohmann::json recv_query();

private:
    std::string address;
    int port;
    SOCKET sockfd;

    nlohmann::json build_info_packet(std::vector<unsigned char>& packet);
    nlohmann::json build_extra_info_packet(std::vector<unsigned char>& packet);
    nlohmann::json build_players_packet(std::vector<unsigned char>& packet);
    nlohmann::json build_rules_packet(std::vector<unsigned char>& packet);
    std::pair<std::string, std::string> decode_buffer(const std::vector<unsigned char>& buf);
};