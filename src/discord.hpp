#pragma once

#include <discord_rpc.h>
#include <ctime>
#include "types.hpp"

namespace Discord
{
    constexpr char DEFAULT_APPLICATION_ID[] = "1324912836291330130";

    void Initialize();
    void Initialize(const char* application_id);
    template <typename T>
    bool IsValidIndex(const std::vector<T> vec, size_t index);
    void Update(const time_t time, const char* state, const char* details, const char* image, const char* imageDetails, const char* infoImage, const char* infoDetails, std::vector<ButtonData> buttons);
    void Update(const time_t time, const std::string& state, const std::string& details, const std::string& image, const std::string& imageDetails, const std::string& infoImage, const std::string& infoDetails, std::vector<ButtonData> buttons);
}