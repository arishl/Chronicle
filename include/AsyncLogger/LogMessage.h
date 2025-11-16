//
// Created by Aris Lemmenes on 11/14/25.
//

#ifndef LFRBLOGGING_LOGMESSAGE_H
#define LFRBLOGGING_LOGMESSAGE_H

#include "LogLevel.h"
#include <chrono>

class LogMessage
{
public:
    LogMessage() = default;
    LogMessage(const LogLevel level, const char* message, const uint32_t thread_id);
    uint64_t timestamp_;
    uint32_t thread_id_;
    LogLevel level_;
    char message_[256];
};

#endif //LFRBLOGGING_LOGMESSAGE_H