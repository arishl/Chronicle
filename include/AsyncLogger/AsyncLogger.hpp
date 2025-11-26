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
#include <thread>

class AsyncLogger
{
public:
    using ThreadID = uint32_t;
    using FileName = std::string;
    explicit AsyncLogger(const FileName& aFilename);
    ~AsyncLogger();
    bool log(const LogMessage& aLogMessage);
    bool log(LogLevel aLevel, const char* aMessage, ThreadID aThreadID);
    void stop();
private:
    using Thread = std::thread;
    using Mutex = std::mutex;
    using AtomicBool = std::atomic<bool>;
    using ConditionVariable = std::condition_variable;
    Allocator mAllocator = Allocator(1024*1024);
    RingBuffer<LogMessage, WaitPolicy::NoWaits, 1000> mBuffer{};
    int mFD;
    LogMessage mCurrentMessage{};
    Thread mWorker;
    AtomicBool mRunning{false};
    AtomicBool mHasData{false};

    struct Line {
        using Length = uint32_t;
        Length mLen;
        char mData[512];
    };
    struct CachedTime {
        uint64_t second = 0;
        char formatted[32] = {0};
    };

    void start();
    static size_t format_timestamp(char* out, uint64_t aTimestampMS);
    void worker_final_check(std::vector<Line>& aLocalBuffer) const;
    void worker_loop();
};

inline size_t write_int(char* aOut, uint32_t aValue, const int aWidth)
{
    for (int i = aWidth - 1; i >= 0; --i)
    {
        aOut[i] = '0' + (aValue % 10);
        aValue /= 10;
    }
    return aWidth;
}

#endif //LFRBLOGGING_ASYNCLOGGER_H