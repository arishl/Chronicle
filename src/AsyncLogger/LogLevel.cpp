//
// Created by Aris Lemmenes on 11/14/25.
//

#include <unordered_map>
#include <string>

#include "../../include/AsyncLogger/LogLevel.h"

struct LevelInfo {
    std::string name;
    std::string color;
};

static std::unordered_map<LogLevel::LevelID, LevelInfo> registry = {
    {0, {"[TRACE]", "\033[37m"}},  // white
    {1, {"[DEBUG]", "\033[36m"}},  // cyan
    {2, {"[INFO]",  "\033[32m"}},  // green
    {3, {"[WARN]",  "\033[33m"}},  // yellow
    {4, {"[ERROR]", "\033[31m"}},  // red
};

void LogLevel::register_level(LevelType v, const std::string& name)
{
    registry[v] = {std::move(name)};
}

const char* LogLevel::to_string(const LogLevel v)
{
    static const char* unknown = "[UNKNOWN]";

    const auto it = registry.find(v.value);
    if (it != registry.end())
        return it->second.name.c_str();

    return unknown;
}

const char* LogLevel::color_of(const LogLevel v)
{
    static const char* no_color = "";
    const auto it = registry.find(v.value);
    if (it != registry.end())
        return it->second.color.c_str();
    return no_color;
}

LogLevel LogLevel::TRACE {0, "[TRACE]"};
LogLevel LogLevel::DEBUG {1, "[DEBUG]"};
LogLevel LogLevel::INFO  {2, "[INFO]"};
LogLevel LogLevel::WARN  {3, "[WARN]"};
LogLevel LogLevel::ERROR {4, "[ERROR]"};