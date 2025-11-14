//
// Created by Aris Lemmenes on 11/14/25.
//

#ifndef LFRBLOGGING_LOGMESSAGE_H
#define LFRBLOGGING_LOGMESSAGE_H

#include "LogLevel.h"
#include <chrono>

struct LogMessage
{
    LogMessage() = default;
    LogMessage(const LogLevel level, const char* message, const uint32_t thread_id) :
        thread_id_ {thread_id}, level_ {level}
    {

        std::strncpy(message_, message, sizeof(message_) - 1);
        message_[sizeof(message_) - 1] = '\0';

        timestamp_ =
            std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
            ).count();
    }

    uint64_t timestamp_;
    uint32_t thread_id_;
    LogLevel level_;
    char message_[256];
};

#endif //LFRBLOGGING_LOGMESSAGE_H