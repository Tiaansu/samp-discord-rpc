#pragma once

#include <string>
#include <vector>
#include <json.hpp>

struct ButtonData
{
    std::string label;
    std::string url;
};

struct ServerData
{
    std::string logo;
    std::string smallImage;
    std::string applicationId;
    std::string largeText;
    std::string smallText;
    std::string details;
    std::string state;
    std::vector<ButtonData> buttons;
};

struct InfoPacket {
    bool password;
    uint16_t players;
    uint16_t max_players;
    std::string hostname;
    std::string gamemode;
    std::string language;

    nlohmann::json to_json() const;
};

struct Player {
    std::string name;
    int32_t score;

    nlohmann::json to_json() const;
};

struct ExtraInfoPacket {
    std::string discord_link;
    std::string light_banner_url;
    std::string dark_banner_url;
    std::string logo_url;

    nlohmann::json to_json() const;
};

inline void from_json(const nlohmann::json& json, ButtonData& button)
{
    if (json.contains("label") && json.contains("url"))
    {
        json.at("label").get_to(button.label);
        json.at("url").get_to(button.url);
        return;
    }
}

inline void from_json(const nlohmann::json& json, ServerData& server)
{
    if (json.contains("logo"))
    {
        json.at("logo").get_to(server.logo);
    }
    if (json.contains("small_image"))
    {
        json.at("small_image").get_to(server.smallImage);
    }
    if (json.contains("application_id"))
    {
        json.at("application_id").get_to(server.applicationId);
    }
    if (json.contains("buttons"))
    {
        json.at("buttons").get_to(server.buttons);
    }
    // placeholders
    if (json.contains("large_text"))
    {
        json.at("large_text").get_to(server.largeText);
    }
    if (json.contains("small_text"))
    {
        json.at("small_text").get_to(server.smallText);
    }
    if (json.contains("details"))
    {
        json.at("details").get_to(server.details);
    }
    if (json.contains("state"))
    {
        json.at("state").get_to(server.state);
    }
}

inline void from_json(const nlohmann::json& json, InfoPacket& result)
{
    if (json.contains("password"))
    {
        json.at("password").get_to(result.password);
    }
    if (json.contains("players"))
    {
        json.at("players").get_to(result.players);
    }
    if (json.contains("max_players"))
    {
        json.at("max_players").get_to(result.max_players);
    }
    if (json.contains("hostname"))
    {
        json.at("hostname").get_to(result.hostname);
    }
    if (json.contains("gamemode"))
    {
        json.at("gamemode").get_to(result.gamemode);
    }
    if (json.contains("language"))
    {
        json.at("language").get_to(result.language);
    }
}

inline void from_json(const nlohmann::json& json, Player& result)
{
    if (json.contains("name"))
    {
        json.at("name").get_to(result.name);
    }
    if (json.contains("score"))
    {
        json.at("score").get_to(result.score);
    }
}

inline void from_json(const nlohmann::json& json, ExtraInfoPacket& result)
{
    if (json.contains("discord_link"))
    {
        json.at("discord_link").get_to(result.discord_link);
    }
    if (json.contains("light_banner_url"))
    {
        json.at("light_banner_url").get_to(result.light_banner_url);
    }
    if (json.contains("dark_banner_url"))
    {
        json.at("dark_banner_url").get_to(result.dark_banner_url);
    }
    if (json.contains("logo_url"))
    {
        json.at("logo_url").get_to(result.logo_url);
    }
}