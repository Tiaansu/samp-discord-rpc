#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#include <process.h>
#include <array>
#include <vector>
#include <iostream>
#include <unordered_map>

#include "client.hpp"
#include "discord.hpp"
#include "http.hpp"
#include "query.hpp"
#include "placeholder.hpp"
#include "types.hpp"

const DWORD SLEEP_TIME = 10000;

static void Process(void*)
{
    SAMP::ServerInfo serverInfo;
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    
    if (!SAMP::ReadServerInfo(GetCommandLine(), serverInfo))
    {
        return;
    }

    std::string serverAddress = serverInfo.address;
    std::string serverPort = serverInfo.port;
    std::string serverUrl = "Tiaansu/samp-discord-rpc/main/servers/" + serverAddress + "_" + serverPort + ".json";

    HTTP::WebRequest webRequest("Mozilla/5.0", "raw.githubusercontent.com", INTERNET_DEFAULT_HTTPS_PORT);
    nlohmann::json serverResponse = webRequest.Get(serverUrl.c_str());

    ServerData serverData;
    try
    {
        serverData = serverResponse.get<ServerData>();
    }
    catch (...) {}

    if (!serverData.applicationId.empty())
    {
        Discord::Initialize(serverData.applicationId.c_str());
    }
    else
    {
        Discord::Initialize();
    }

    Query serverQuery;
    if (!serverQuery.new_query(serverAddress, std::stoi(serverPort)))
    {
        return;
    }

    ExtraInfoPacket extraInfo;
    serverQuery.send_query('r');

    nlohmann::json queryResponse = serverQuery.recv_query();
    if ((queryResponse.contains("error") && queryResponse["error"] != "timeout") || queryResponse.contains("ping"))
    {
        return;
    }
    if (queryResponse.contains("version") && queryResponse["version"].find("omp") != queryResponse["version"].end())
    {
        serverQuery.send_query('o');

        nlohmann::json extraInfoResponse = serverQuery.recv_query();
        if (extraInfoResponse.contains("error") || extraInfoResponse.contains("ping"))
        {
            extraInfo.logo_url = serverData.logo;
        }
        else
        {
            extraInfo = extraInfoResponse.get<ExtraInfoPacket>();
        }
    }
    else
    {
        extraInfo.logo_url = serverData.logo;
    }

    if (extraInfo.logo_url.empty())
    {
        extraInfo.logo_url = "logo";
    }

    auto startTime = std::time(0);
    if (serverInfo.connect == SAMP::SAMP_CONNECT_SERVER)
    {
        while (true)
        {
            if (serverQuery.send_query('i') <= 0)
            {
                Sleep(SLEEP_TIME);
                continue;
            }

            nlohmann::json infoResponse = serverQuery.recv_query();
            if (infoResponse.contains("error") || infoResponse.contains("ping"))
            {
                Sleep(SLEEP_TIME);
                continue;
            }

            InfoPacket infoPacket = infoResponse.get<InfoPacket>();
            
            std::unordered_map<std::string, std::any> placeholders = 
            {
                {"SERVER_NAME", infoPacket.hostname},
                {"SERVER_IP", serverInfo.address},
                {"SERVER_PORT", serverInfo.port},
                {"USERNAME", serverInfo.username},
                {"PLAYERS", infoPacket.players},
                {"MAX_PLAYERS", infoPacket.max_players},
                {"LANGUAGE", infoPacket.language},
                {"GAMEMODE", infoPacket.gamemode}
            };
            
            auto fullAddress = hasPlaceholder(serverData.state) ? replacePlaceholders(serverData.state, placeholders) : serverInfo.address + ':' + serverInfo.port;
            auto playersOnline = hasPlaceholder(serverData.largeText) 
                ? replacePlaceholders(serverData.largeText, placeholders) 
                : std::to_string(infoPacket.players) + "/" + std::to_string(infoPacket.max_players) + " players online";
            auto infoText = hasPlaceholder(serverData.smallText)
                ? replacePlaceholders(serverData.smallText, placeholders)
                : "Playing " + infoPacket.gamemode + " as " + serverInfo.username + " in " + infoPacket.language;

            if (Discord::IsValidIndex(serverData.buttons, 0))
            {
                if (hasPlaceholder(serverData.buttons[0].label))
                {
                    serverData.buttons[0].label = replacePlaceholders(serverData.buttons[0].label, placeholders);
                }
                if (hasPlaceholder(serverData.buttons[0].url))
                {
                    serverData.buttons[0].url = replacePlaceholders(serverData.buttons[0].url, placeholders);
                }
            }
            else
            {
                ButtonData button;
                button.label = "Join";
                button.url = "samp://" + serverInfo.address + ":" + serverInfo.port;
                serverData.buttons.push_back(button);
            }

            if (Discord::IsValidIndex(serverData.buttons, 1))
            {
                if (hasPlaceholder(serverData.buttons[1].label))
                {
                    serverData.buttons[1].label = replacePlaceholders(serverData.buttons[1].label, placeholders);
                }
                if (hasPlaceholder(serverData.buttons[1].url))
                {
                    serverData.buttons[1].url = replacePlaceholders(serverData.buttons[1].url, placeholders);
                }
            }
            else
            {
                ButtonData button;

                serverQuery.send_query('o');

                nlohmann::json extraInfoResponse = serverQuery.recv_query();
                if (!extraInfoResponse.contains("error") && !extraInfoResponse.contains("ping"))
                {
                    ExtraInfoPacket extraInfo = extraInfoResponse.get<ExtraInfoPacket>();
                    if (!extraInfo.discord_link.empty())
                    {
                        button.label = "Discord";
                        button.url = extraInfo.discord_link;
                    }
                }
            }

            Discord::Update(startTime, fullAddress, infoPacket.hostname, extraInfo.logo_url, infoText, serverData.smallImage, playersOnline, serverData.buttons);
            Sleep(SLEEP_TIME);
        }
    }
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
        {
            DisableThreadLibraryCalls(module);
            _beginthread(&Process, 0, nullptr);
            break;
        }
        case DLL_PROCESS_DETACH:
        {
            WSACleanup();
            break;
        }
    }
    return TRUE;
}