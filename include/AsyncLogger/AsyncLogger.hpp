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
#include "../Allocator/Allocator.hpp"

class AsyncLogger
{
public:
    explicit AsyncLogger(const std::string& aFilename);
    ~AsyncLogger();

    bool log(const LogMessage& aLogMessage);
    bool log(LogLevel aLevel, const char* aMessage, uint32_t aThreadID);
    void start();
    void stop();
private:
    Allocator mAllocator = Allocator(4096);
    RingBuffer<LogMessage, WaitPolicy::NoWaits, 4096> mBuffer{};
    int mFD;
    LogMessage mCurrentMessage{};
    std::thread mWorker;
    std::atomic<bool> mRunning{false};
    std::mutex mMTX;
    std::condition_variable mCV;

    struct Line {
        uint16_t mLen;
        char mData[512];
    };

    static size_t format_timestamp(char* out, uint64_t timestampMS);
    void worker_final_check(std::vector<Line>& aLocalBuffer) const;
    void worker_loop();
};

#endif //LFRBLOGGING_ASYNCLOGGER_H