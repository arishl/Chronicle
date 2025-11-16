//
// Created by Aris Lemmenes on 11/15/25.
//
#include "../../include/AsyncLogger/LogMessage.h"
LogMessage::LogMessage(const LogLevel level, const char* message, const uint32_t thread_id) :
        thread_id_ {thread_id}, level_ {level}
{

    std::strncpy(message_, message, sizeof(message_) - 1);
    message_[sizeof(message_) - 1] = '\0';

    timestamp_ =
        std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
        ).count();
}