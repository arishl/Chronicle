//
// Created by Aris Lemmenes on 11/15/25.
//
#include "../../include/AsyncLogger/LogMessage.hpp"
#include <chrono>

LogMessage::LogMessage(const LogLevel aLevel, const char* aMessage,
                       const ThreadID aThreadID) :
    mThreadID{aThreadID}, mLevel{aLevel}, mMessage{} {
    std::strncpy(mMessage, aMessage, sizeof(mMessage) - 1);
    mMessage[sizeof(mMessage) - 1] = '\0';
    mTimestamp =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
}
