//
// Created by Aris Lemmenes on 11/12/25.
//

#ifndef LFRBLOGGING_ASYNCLOGGER_H
#define LFRBLOGGING_ASYNCLOGGER_H
#include <string>
#include <fstream>
#include "../RingBuffer/RingBuffer.hpp"
#include <thread>
#include <atomic>
#include <chrono>
#include "LogLevel.hpp"
#include "LogMessage.hpp"

class AsyncLogger
{
public:
    explicit AsyncLogger(const std::string& filename);
    ~AsyncLogger();

    bool log(const LogMessage& logMessage);
    bool log(LogLevel level, const char* message, uint32_t threadID);
    void start();
    void stop();
private:
    RingBuffer<LogMessage, 4096, ThreadsPolicy::MPMC, WaitPolicy::BothWait> buffer_;
    int mFD;
    LogMessage mCurrentMessage{};
    std::thread mWorker;
    std::atomic<bool> mRunning{false};
    std::mutex mMTX;
    std::condition_variable mCV;

    struct Line {
        uint16_t len;
        char data[512];
    };

    static size_t format_timestamp(char* out, uint64_t timestampMS);
    void worker_final_check(std::vector<Line>& localBuffer) const;
    void worker_loop();
};

#endif //LFRBLOGGING_ASYNCLOGGER_H