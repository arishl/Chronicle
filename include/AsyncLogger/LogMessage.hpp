//
// Created by Aris Lemmenes on 11/14/25.
//

#ifndef LFRBLOGGING_LOGMESSAGE_H
#define LFRBLOGGING_LOGMESSAGE_H

#include "LogLevel.hpp"
#include <chrono>

struct LogMessage
{
    LogMessage() = default;
    LogMessage(const LogLevel level, const char* message, const uint32_t threadID);
    uint64_t mTimestamp;
    uint32_t mThreadID;
    LogLevel mLevel;
    char mMessage[256];
};

#endif //LFRBLOGGING_LOGMESSAGE_H