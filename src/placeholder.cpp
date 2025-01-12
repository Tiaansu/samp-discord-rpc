#include "placeholder.hpp"

static std::regex placeholderRegex("\\{([A-Z_]+)\\}");

std::string anyToString(const std::any& value)
{
    if (value.type() == typeid(std::string))
    {
        return std::any_cast<std::string>(value);
    }
    else if (value.type() == typeid(int))
    {
        return std::to_string(std::any_cast<int>(value));
    }
    else if (value.type() == typeid(double))
    {
        return std::to_string(std::any_cast<double>(value));
    }
    else if (value.type() == typeid(bool))
    {
        return std::any_cast<bool>(value) ? "true" : "false";
    }
    else if (value.type() == typeid(uint16_t))
    {
        return std::to_string(std::any_cast<uint16_t>(value));
    }
    return "";
}

std::string replacePlaceholders(const std::string& text, const std::unordered_map<std::string, std::any>& placeholders)
{
    std::string result = text;

    for (std::sregex_iterator it(text.begin(), text.end(), placeholderRegex), end; it != end; ++ it)
    {
        std::smatch match = *it;
        std::string placeholderName = match[1].str();

        auto placeholderValueIt = placeholders.find(placeholderName);
        if (placeholderValueIt != placeholders.end())
        {
            const std::any& anyValue = placeholderValueIt->second;
            std::string replacementValue = anyToString(anyValue);
            result = std::regex_replace(result, std::regex("\\{" + placeholderName + "\\}"), replacementValue);
        }
    }
    return result;
}

bool hasPlaceholder(const std::string& text)
{
    return std::regex_search(text, placeholderRegex);
}