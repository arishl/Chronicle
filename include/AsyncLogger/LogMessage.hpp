//
// Created by Aris Lemmenes on 11/14/25.
//

#ifndef LFRBLOGGING_LOGMESSAGE_H
#define LFRBLOGGING_LOGMESSAGE_H

#include "LogLevel.hpp"
#include <chrono>

struct LogMessage {
    using TimeStamp = uint64_t;
    using ThreadID = uint32_t;
    LogMessage() = default;
    LogMessage(LogLevel aLevel, const char* aMessage, ThreadID aThreadID);
    TimeStamp mTimestamp;
    ThreadID mThreadID;
    LogLevel mLevel;
    char mMessage[256];
};

#endif //LFRBLOGGING_LOGMESSAGE_H
