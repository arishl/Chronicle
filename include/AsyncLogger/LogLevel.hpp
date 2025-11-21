//
// Created by Aris Lemmenes on 11/14/25.
//

#ifndef LFRBLOGGING_LOGLEVEL_H
#define LFRBLOGGING_LOGLEVEL_H

#include <unordered_map>

class LogLevel {
public:
    using LevelID = uint32_t;
    using LevelType = uint32_t;
    LogLevel() = default;
    explicit LogLevel(const LevelType value, const std::string& name)
        : mValue(value)
    {
        register_level(value, name);
    }
    static void register_level(LevelType aValue, const std::string& aName);
    static const char* to_string(LogLevel aValue);
    static const char* color_of(LogLevel value);

    static LogLevel TRACE;
    static LogLevel DEBUG;
    static LogLevel INFO;
    static LogLevel WARN;
    static LogLevel ERROR;
private:
    LevelType mValue;
};



#endif //LFRBLOGGING_LOGLEVEL_H