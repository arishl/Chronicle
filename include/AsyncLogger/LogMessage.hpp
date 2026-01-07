//
// Created by Aris Lemmenes on 11/14/25.
//

#ifndef LFRBLOGGING_LOGMESSAGE_H
#define LFRBLOGGING_LOGMESSAGE_H

#include "LogLevel.hpp"

struct alignas(64) LogMessage {
    using TimeStamp = uint64_t;
    using ThreadID = uint32_t;
    LogMessage() = default;
    LogMessage(LogLevel aLevel, const char* aMessage, ThreadID aThreadID);
    TimeStamp mTimestamp;
    char mMessage[256];
    ThreadID mThreadID;
    LogLevel mLevel;
};

#endif //LFRBLOGGING_LOGMESSAGE_H
