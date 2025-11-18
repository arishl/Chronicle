//
// Created by Aris Lemmenes on 11/15/25.
//
#include "../../include/AsyncLogger/LogMessage.hpp"
LogMessage::LogMessage(const LogLevel level, const char* message, const uint32_t threadID) :
        thread_id_ {threadID}, mLevel {level}
{

    std::strncpy(mMessage, message, sizeof(mMessage) - 1);
    mMessage[sizeof(mMessage) - 1] = '\0';

    mTimestamp =
        std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
        ).count();
}