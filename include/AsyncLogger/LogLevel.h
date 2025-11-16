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
    explicit LogLevel(const LevelType v, const std::string& name)
        : value(v)
    {
        register_level(v, name);
    }
    static void register_level(LevelType v, const std::string& name);
    static const char* to_string(LogLevel v);
    static const char* color_of(LogLevel v);

    static LogLevel TRACE;
    static LogLevel DEBUG;
    static LogLevel INFO;
    static LogLevel WARN;
    static LogLevel ERROR;
private:
    LevelType value;
};



#endif //LFRBLOGGING_LOGLEVEL_H