#include "discord.hpp"

namespace Discord
{
    const char* currentApplicationId;

    static void Ready(const DiscordUser* user) {}
    static void Errored(int errorCode, const char* message) {}
    static void Disconnected(int errorCode, const char* message) {}
    static void JoinGame(const char* joinSecret) {}
    static void SpectateGame(const char* spectateSecret) {}
    static void JoinRequest(const DiscordUser* user) {}

    void Initialize()
    {
        Initialize(DEFAULT_APPLICATION_ID);
    }
    void Initialize(const char* application_id)
    {
        DiscordEventHandlers handlers = { 0 };
        handlers.ready = Ready;
        handlers.errored = Errored;
        handlers.disconnected = Disconnected;
        handlers.joinGame = JoinGame;
        handlers.spectateGame = SpectateGame;
        handlers.joinRequest = JoinRequest;
        currentApplicationId = application_id;
        Discord_Initialize(application_id, &handlers, 1, nullptr);
        Discord_ClearPresence();
    }

    void Update(const time_t time, const char* state, const char* details, const char* image, const char* imageDetails, const char* infoImage, const char* infoDetails, std::vector<ButtonData> buttons)
    {
        DiscordRichPresence presence = { 0 };
        presence.state = state;
        presence.details = details;
        presence.startTimestamp = time;
        presence.largeImageKey = image;
        presence.largeImageText = imageDetails;
        presence.smallImageKey = strlen(infoImage) > 0 ? infoImage : (!strcmp(currentApplicationId, DEFAULT_APPLICATION_ID)) ? "info" : image;
        presence.smallImageText = infoDetails;

        if (IsValidIndex(buttons, 0))
        {
            presence.button1Label = buttons[0].label.c_str();
            presence.button1Url = buttons[0].url.c_str();
        }
        if (IsValidIndex(buttons, 1))
        {
            presence.button2Label = buttons[1].label.c_str();
            presence.button2Url = buttons[1].url.c_str();
        }
        
        Discord_UpdatePresence(&presence);
    }

    void Update(const time_t time, const std::string& state, const std::string& details, const std::string& image, const std::string& imageDetails, const std::string& infoImage, const std::string& infoDetails, std::vector<ButtonData> buttons)
    {
        return Update(time, state.c_str(), details.c_str(), image.c_str(), imageDetails.c_str(), infoImage.c_str(), infoDetails.c_str(), buttons);
    }

    template <typename T>
    bool IsValidIndex(const std::vector<T> vec, size_t index)
    {
        return index < vec.size();
    }
}