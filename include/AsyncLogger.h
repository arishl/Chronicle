//
// Created by Aris Lemmenes on 11/12/25.
//

#ifndef LFRBLOGGING_ASYNCLOGGER_H
#define LFRBLOGGING_ASYNCLOGGER_H
#include <string>
#include <chrono>
#include <fstream>
#include "RingBuffer.h"
#include <thread>
#include <atomic>
enum class LogLevel
{
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
};

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

class AsyncLogger
{
public:
    explicit AsyncLogger(const std::string& filename);
    ~AsyncLogger();

    bool log(const LogMessage& log_message);
    void start();
    void stop();
private:
    RingBuffer<LogMessage, 4096> buffer_;
    std::ofstream file_;
    LogMessage current_msg_;
    std::thread worker_;
    std::atomic<bool> running_{false};
    std::mutex mtx_;
    std::condition_variable cv_;

    void worker_loop();

    static std::string level_to_string(LogLevel level);
    static std::string format_timestamp(uint64_t timestamp_ms);
};

#endif //LFRBLOGGING_ASYNCLOGGER_H