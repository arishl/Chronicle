//
// Created by Aris Lemmenes on 11/14/25.
//

#include <unordered_map>
#include <string>

#include "../include/AsyncLogger/LogLevel.h"

static std::unordered_map<LogLevel::LevelID, std::string> registry = {
    {0, "[TRACE]"},
    {1, "[DEBUG]"},
    {2, "[INFO]"},
    {3, "[WARN]"},
    {4, "[ERROR]"},
};

void LogLevel::register_level(LevelType v, std::string name) {
    registry[v] = std::move(name);
}

const char* LogLevel::to_string(const LogLevel v)
{
    static const char* unknown = "[UNKNOWN]";

    if (const auto it = registry.find(v.value); it != registry.end()) {
        return it->second.c_str();
    }

    return unknown;
}

LogLevel LogLevel::TRACE {0, "[TRACE]"};
LogLevel LogLevel::DEBUG {1, "[DEBUG]"};
LogLevel LogLevel::INFO {2, "[INFO]"};
LogLevel LogLevel::WARN {3, "[WARN]"};
LogLevel LogLevel::ERROR {4, "[ERROR]"};

