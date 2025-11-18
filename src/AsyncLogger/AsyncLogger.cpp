//
// Created by Aris Lemmenes on 11/12/25.
//
#include <fstream>
#include <unistd.h>
#include <fcntl.h>

#include "../../include/AsyncLogger/AsyncLogger.hpp"


AsyncLogger::AsyncLogger(const std::string& filename) : buffer_()
{
    mFD = ::open(filename.c_str(),
                 O_WRONLY | O_CREAT | O_APPEND,
                 0644);

    if (mFD < 0)
    {
        throw std::runtime_error("Failed to open log file");
    }

    mRunning = true;
}

AsyncLogger::~AsyncLogger()
{
    stop();
    if (mFD >= 0) {
        ::close(mFD);
    }
}

bool AsyncLogger::log(const LogMessage& logMessage)
{
    const bool pushed = buffer_.push(logMessage);
    if (pushed)
        mCV.notify_one();
    return pushed;
}

bool AsyncLogger::log(const LogLevel level, const char* message, const uint32_t threadID)
{
    const LogMessage msg(level, message, threadID);
    const bool pushed = log(msg);
    if (pushed)
        mCV.notify_one();
    return pushed;
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

    const auto tp = time_point<system_clock, milliseconds>(milliseconds(timestampMS));
    std::time_t t = system_clock::to_time_t(tp);

    std::tm tm {};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    int n = std::snprintf(
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

    return static_cast<size_t>(n);
}

void AsyncLogger::worker_final_check(std::vector<Line>& localBuffer) const
{
    if (!localBuffer.empty())
    {
        std::string big;
        size_t capacity = 0;
        for (auto& [len, data] : localBuffer)
        {
            capacity += len;
        }
        big.reserve(capacity);
        for (auto& [len, data] : localBuffer)
        {
            big.append(data, len);
        }
        ::write(mFD, big.data(), big.size());
    }
}

void AsyncLogger::worker_loop()
{
    static constexpr size_t MAX_BATCH_BYTES = 32 * 1024;
    std::vector<Line> local_buffer;
    local_buffer.reserve(512);
    size_t batch_bytes = 0;
    while (mRunning || !buffer_.empty() || !local_buffer.empty())
    {
        LogMessage msg {};
        const bool got_msg = buffer_.pop(msg);
        if (got_msg)
        {
            char ts[32];
            const size_t ts_len = format_timestamp(ts, msg.mTimestamp);
            const char* level_str = LogLevel::to_string(msg.mLevel);
            const char* level_color = LogLevel::color_of(msg.mLevel);
            Line& line = local_buffer.emplace_back();
            /**
            const int n = std::snprintf(
                line.data,
                sizeof(line.data),
                "%.*s %s%s\033[0m [T%u] %s\n",
                static_cast<int>(ts_len),
                ts,
                level_color,
                level_str,
                msg.thread_id_,
                msg.message_
            );
            **/
            const int n = std::snprintf(
                line.data,
                sizeof(line.data),
                "%.*s %s [T%u] %s\n",
                static_cast<int>(ts_len), ts,
                level_str,
                msg.thread_id_,
                msg.mMessage
            );
            line.len = static_cast<uint16_t>(n);
            batch_bytes += n;
        }
        if (const bool batch_full = batch_bytes >= MAX_BATCH_BYTES; !local_buffer.empty() && (batch_full || (!mRunning && buffer_.empty())))
        {
            std::string big;
            big.reserve(batch_bytes);
            for (auto& l : local_buffer)
            {
                big.append(l.data, l.len);
            }
            ::write(mFD, big.data(), big.size());
            local_buffer.clear();
            batch_bytes = 0;
        }
        if (!got_msg)
        {
            std::unique_lock<std::mutex> lock(mMTX);
            mCV.wait(lock, [&] {
                return !buffer_.empty() || !mRunning;
            });
        }
    }

    worker_final_check(local_buffer);

}


