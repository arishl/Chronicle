//
// Created by Aris Lemmenes on 11/12/25.
//
#include <fstream>
#include <unistd.h>
#include <fcntl.h>

#include "../../include/AsyncLogger/AsyncLogger.hpp"


AsyncLogger::AsyncLogger(const std::string& aFilename)
{
    mFD = ::open(aFilename.c_str(),
                 O_WRONLY | O_CREAT | O_APPEND,
                 0644);

    if (mFD < 0)
    {
        throw std::runtime_error("Failed to open log file");
    }

    mRunning = true;
    mAllocator.allocate(100, 64);
}

AsyncLogger::~AsyncLogger()
{
    stop();
    if (mFD >= 0) {
        ::close(mFD);
    }
}

bool AsyncLogger::log(const LogMessage& aLogMessage)
{
    bool cPushed {true};
    cPushed = mBuffer.emplace(aLogMessage);
    if (cPushed)
    {
        mCV.notify_one();
    }
    return cPushed;
}

bool AsyncLogger::log(const LogLevel aLevel, const char* aMessage, const uint32_t aThreadID)
{
    const LogMessage cMsg(aLevel, aMessage, aThreadID);
    const bool cPushed = log(cMsg);
    if (cPushed)
        mCV.notify_one();
    return cPushed;
}

void AsyncLogger::start()
{
    mRunning = true;
    mWorker = std::thread(&AsyncLogger::worker_loop, this);
}

void AsyncLogger::stop()
{
    mRunning = false;
    mCV.notify_one();
    if (mWorker.joinable())
        mWorker.join();
}

size_t AsyncLogger::format_timestamp(char* out, uint64_t timestampMS)
{
    using namespace std::chrono;

    const auto cTP = time_point<system_clock, milliseconds>(milliseconds(timestampMS));
    std::time_t t = system_clock::to_time_t(cTP);

    std::tm tm {};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    const int cPrintable = std::snprintf(
        out,
        32,
        "%04d-%02d-%02d %02d:%02d:%02d.%03llu",
        tm.tm_year + 1900,
        tm.tm_mon + 1,
        tm.tm_mday,
        tm.tm_hour,
        tm.tm_min,
        tm.tm_sec,
        static_cast<unsigned long long>(timestampMS % 1000)
    );

    return static_cast<size_t>(cPrintable);
}

void AsyncLogger::worker_final_check(std::vector<Line>& aLocalBuffer) const
{
    if (!aLocalBuffer.empty())
    {
        std::string cBig;
        size_t cCapacity = 0;
        for (auto& [len, data] : aLocalBuffer)
        {
            cCapacity += len;
        }
        cBig.reserve(cCapacity);
        for (auto& [len, data] : aLocalBuffer)
        {
            cBig.append(data, len);
        }
        ::write(mFD, cBig.data(), cBig.size());
    }
}

void AsyncLogger::worker_loop()
{
    static constexpr size_t MAX_BATCH_BYTES = 32 * 1024;
    std::vector<Line> cLocalBuffer;
    cLocalBuffer.reserve(512);
    size_t cBatchBytes = 0;
    while (mRunning || !mBuffer.empty() || !cLocalBuffer.empty())
    {
        LogMessage cMsg {};
        bool cGotMsg = true;
        cGotMsg = mBuffer.pop(cMsg);
        if (cGotMsg)
        {
            char cTS[32];
            const size_t cTSLen = format_timestamp(cTS, cMsg.mTimestamp);
            const char* cLevelStr = LogLevel::to_string(cMsg.mLevel);
            const char* cLevelColor = LogLevel::color_of(cMsg.mLevel);
            Line& cLine = cLocalBuffer.emplace_back();
            const int n = std::snprintf(
                cLine.mData,
                sizeof(cLine.mData),
                "%.*s %s [T%u] %s\n",
                static_cast<int>(cTSLen), cTS,
                cLevelStr,
                cMsg.mThreadID,
                cMsg.mMessage
            );
            cLine.mLen = static_cast<uint16_t>(n);
            cBatchBytes += n;
        }
        if (const bool cBatchFull = cBatchBytes >= MAX_BATCH_BYTES; !cLocalBuffer.empty() && (cBatchFull || (!mRunning && mBuffer.empty())))
        {
            std::string cBig;
            cBig.reserve(cBatchBytes);
            for (auto& l : cLocalBuffer)
            {
                cBig.append(l.mData, l.mLen);
            }
            ::write(mFD, cBig.data(), cBig.size());
            cLocalBuffer.clear();
            cBatchBytes = 0;
        }
        if (!cGotMsg)
        {
            std::unique_lock<std::mutex> lock(mMTX);
            mCV.wait(lock, [&] {
                return !mBuffer.empty() || !mRunning;
            });
        }
    }

    worker_final_check(cLocalBuffer);

}


