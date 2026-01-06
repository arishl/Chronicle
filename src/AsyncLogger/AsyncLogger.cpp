//
// Created by Aris Lemmenes on 11/12/25.
//
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/uio.h>

#include "../../include/AsyncLogger/AsyncLogger.hpp"


AsyncLogger::AsyncLogger(const FileName& aFilename) {
    mFD = open(aFilename.c_str(),
               O_WRONLY | O_CREAT | O_APPEND,
               0644);
    if (mFD < 0) {
        throw std::runtime_error("Failed to open log file");
    }
    mBuffer.allocate(mAllocator);
    start();
}

AsyncLogger::~AsyncLogger() {
    stop();
    if (mFD >= 0) {
        close(mFD);
    }
}

bool AsyncLogger::log(const LogMessage& aLogMessage) {
    const bool cPushed{mBuffer.emplace(aLogMessage)};
    if (cPushed) {
        mHasData.store(true, std::memory_order_release);
    }
    return cPushed;
}

bool AsyncLogger::log(const LogLevel aLevel, const char* aMessage,
                      const ThreadID aThreadID) {
    const LogMessage cMsg(aLevel, aMessage, aThreadID);
    const bool cPushed = log(cMsg);
    {
        mHasData.store(true, std::memory_order_release);
    }
    return cPushed;
}

void AsyncLogger::start() {
    if (mRunning == false) {
        mRunning = true;
        mWorker = std::thread(&AsyncLogger::worker_loop, this);
    }
}

void AsyncLogger::stop() {
    mRunning = false;
    if (mWorker.joinable())
        mWorker.join();
}

size_t AsyncLogger::format_timestamp(char* aOut, uint64_t aTimestampMS) {
    // thread-local cached time per thread
    thread_local CachedTime cCachedTime;

    const uint64_t seconds = aTimestampMS / 1000;
    const uint64_t ms = aTimestampMS % 1000;

    if (cCachedTime.second != seconds) {
        cCachedTime.second = seconds;

        auto t = static_cast<time_t>(seconds);
        std::tm tm{};
#if defined(_WIN32)
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif

        char* p = cCachedTime.formatted;
        p += write_int(p, tm.tm_year + 1900, 4);
        *p++ = '-';
        p += write_int(p, tm.tm_mon + 1, 2);
        *p++ = '-';
        p += write_int(p, tm.tm_mday, 2);
        *p++ = ' ';
        p += write_int(p, tm.tm_hour, 2);
        *p++ = ':';
        p += write_int(p, tm.tm_min, 2);
        *p++ = ':';
        p += write_int(p, tm.tm_sec, 2);
        *p = '\0';
    }

    char* p = aOut;
    const char* src = cCachedTime.formatted;
    while (*src)
        *p++ = *src++;

    *p++ = '.';
    p += write_int(p, static_cast<uint32_t>(ms), 3);

    return p - aOut;
}

void AsyncLogger::worker_final_check(std::vector<Line>& aLocalBuffer) const {
    if (!aLocalBuffer.empty()) {
        std::string cBig;
        size_t cCapacity = 0;
        for (auto& [len, data] : aLocalBuffer) {
            cCapacity += len;
        }
        cBig.reserve(cCapacity);
        for (auto& [len, data] : aLocalBuffer) {
            cBig.append(data, len);
        }
        ::write(mFD, cBig.data(), cBig.size());
    }
}

void AsyncLogger::worker_loop() {
    static constexpr size_t MAX_BATCH_BYTES = 32 * 1024;
    std::vector<Line> cLocalBuffer;
    cLocalBuffer.reserve(512);
    size_t cBatchBytes = 0;
    while (mRunning || !mBuffer.empty() || !cLocalBuffer.empty()) {
        LogMessage cMsg{};
        const bool cGotMsg = mBuffer.pop(cMsg);
        if (cGotMsg) {
            char cTS[32];
            const size_t cTSLen = format_timestamp(cTS, cMsg.mTimestamp);
            const char* cLevelStr = LogLevel::to_string(cMsg.mLevel);
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
        if (const bool cBatchFull = cBatchBytes >= MAX_BATCH_BYTES; !
            cLocalBuffer.empty() && (cBatchFull || (!mRunning && mBuffer.
                empty()))) {
            std::vector<iovec> cIOVec;
            cIOVec.reserve(cLocalBuffer.size());
            for (auto& [mLen, mData] : cLocalBuffer) {
                iovec ioVec{};
                ioVec.iov_base = mData;
                ioVec.iov_len = mLen;
                cIOVec.push_back(ioVec);
            }
            writev(mFD, cIOVec.data(), static_cast<int>(cIOVec.size()));
            cLocalBuffer.clear();
            cBatchBytes = 0;
        }
        if (!cGotMsg) {
            if (!mHasData.load(std::memory_order_acquire)) {
                for (int i = 0; i < 100; ++i) {
                    if (mHasData.load(std::memory_order_acquire))
                        break;
                }
                if (!mHasData.load(std::memory_order_acquire))
                    std::this_thread::sleep_for(std::chrono::microseconds(30));
            }
            continue;
        }
    }

    worker_final_check(cLocalBuffer);
}
