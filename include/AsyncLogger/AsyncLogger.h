//
// Created by Aris Lemmenes on 11/12/25.
//

#ifndef LFRBLOGGING_ASYNCLOGGER_H
#define LFRBLOGGING_ASYNCLOGGER_H
#include <string>
#include <fstream>
#include "../RingBuffer/RingBuffer.h"
#include <thread>
#include <atomic>
#include <chrono>
#include "LogLevel.h"
#include "LogMessage.h"

class AsyncLogger
{
public:
    explicit AsyncLogger(const std::string& filename);
    ~AsyncLogger();

    bool log(const LogMessage& log_message);
    bool log(LogLevel level, const char* message, uint32_t thread_id);
    void start();
    void stop();
private:
    RingBuffer<LogMessage, 4096> buffer_;
    int fd_;
    LogMessage current_msg_{};
    std::thread worker_;
    std::atomic<bool> running_{false};
    std::mutex mtx_;
    std::condition_variable cv_;

    struct Line {
        uint16_t len;
        char data[512];
    };

    static const char* level_to_string(LogLevel level);
    static size_t format_timestamp(char* out, uint64_t timestamp_ms);
    void worker_final_check(std::vector<Line>& local_buffer) const;
    void worker_loop();
};

#endif //LFRBLOGGING_ASYNCLOGGER_H