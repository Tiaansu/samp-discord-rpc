#include <string>
#include <regex>
#include <any>
#include <unordered_map>

std::string anyToString(const std::any& value);
std::string replacePlaceholders(const std::string& text, const std::unordered_map<std::string, std::any>& placeholders);
bool hasPlaceholder(const std::string& text);