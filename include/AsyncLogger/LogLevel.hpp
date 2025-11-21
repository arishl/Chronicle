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
    using LevelName = std::string;
    LogLevel() = default;
    explicit LogLevel(LevelType aValue, const LevelName& aName);
    static void register_level(LevelType aValue, const LevelName& aName);
    static const char* to_string(LogLevel aValue);
    static const char* color_of(LogLevel aValue);

    static LogLevel TRACE;
    static LogLevel DEBUG;
    static LogLevel INFO;
    static LogLevel WARN;
    static LogLevel ERROR;

private:
    LevelType mValue;
};



#endif //LFRBLOGGING_LOGLEVEL_H