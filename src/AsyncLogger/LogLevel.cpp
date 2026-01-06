//
// Created by Aris Lemmenes on 11/14/25.
//

#include <unordered_map>
#include <string>

#include "../../include/AsyncLogger/LogLevel.hpp"

struct LevelInfo {
    std::string mName;
    std::string mColor;
};

static std::unordered_map<LogLevel::LevelID, LevelInfo> sRegistry = {
    {0, {"[TRACE]", "\033[37m"}}, // white
    {1, {"[DEBUG]", "\033[36m"}}, // cyan
    {2, {"[INFO]", "\033[32m"}}, // green
    {3, {"[WARN]", "\033[33m"}}, // yellow
    {4, {"[ERROR]", "\033[31m"}}, // red
};

LogLevel::LogLevel(const LevelType aValue, const LevelName& aName)
    : mValue(aValue) {
    register_level(aValue, aName);
}

void LogLevel::register_level(const LevelType aValue, const LevelName& aName) {
    sRegistry[aValue] = {aName};
}

const char* LogLevel::to_string(const LogLevel aValue) {
    static auto sUnknown = "[UNKNOWN]";
    if (const auto it = sRegistry.find(aValue.mValue); it != sRegistry.end())
        return it->second.mName.c_str();
    return sUnknown;
}

const char* LogLevel::color_of(const LogLevel aValue) {
    static auto sNoColor = "";
    if (const auto it = sRegistry.find(aValue.mValue); it != sRegistry.end())
        return it->second.mColor.c_str();
    return sNoColor;
}

LogLevel LogLevel::TRACE{0, "[TRACE]"};
LogLevel LogLevel::DEBUG{1, "[DEBUG]"};
LogLevel LogLevel::INFO{2, "[INFO]"};
LogLevel LogLevel::WARN{3, "[WARN]"};
LogLevel LogLevel::ERROR{4, "[ERROR]"};
