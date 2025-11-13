//
// Created by Aris Lemmenes on 11/12/25.
//
#include <fstream>
#include <unistd.h>
#include <fcntl.h>


#include "../include/AsyncLogger.h"


AsyncLogger::AsyncLogger(const std::string& filename)
{
    fd_ = ::open(filename.c_str(),
                 O_WRONLY | O_CREAT | O_APPEND,
                 0644);

    if (fd_ < 0) {
        throw std::runtime_error("Failed to open log file");
    }

    running_ = true;
}

AsyncLogger::~AsyncLogger()
{
    stop();
    if (fd_ >= 0) {
        ::close(fd_);
    }
}

bool AsyncLogger::log(const LogMessage& msg)
{
    bool pushed = buffer_.push(msg);
    if (pushed)
        cv_.notify_one();
    return pushed;
}

void AsyncLogger::start()
{
    running_ = true;
    worker_ = std::thread(&AsyncLogger::worker_loop, this);
}

void AsyncLogger::stop()
{
    running_ = false;
    cv_.notify_one();
    if (worker_.joinable())
        worker_.join();
}

char* AsyncLogger::level_to_string(const LogLevel level)
{
    switch (level)
    {
    case LogLevel::TRACE: return "[TRACE]";
    case LogLevel::DEBUG: return "[DEBUG]";
    case LogLevel::INFO:  return "[INFO]";
    case LogLevel::WARN:  return "[WARN]";
    case LogLevel::ERROR: return "[ERROR]";
    }
    return "[UNKNOWN]";
}

size_t AsyncLogger::format_timestamp(char* out, uint64_t timestamp_ms)
{
    using namespace std::chrono;

    // Convert timestamp_ms → time_t seconds
    const auto tp = time_point<system_clock, milliseconds>(milliseconds(timestamp_ms));
    std::time_t t = system_clock::to_time_t(tp);

    // Convert to calendar time
    std::tm tm;
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);  // thread-safe
#endif

    // Format YYYY-MM-DD HH:MM:SS into buffer
    // Example: "2025-11-13 14:03:51"
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
        static_cast<unsigned long long>(timestamp_ms % 1000)
    );

    return static_cast<size_t>(n);
}

void AsyncLogger::worker_loop()
{
    // Local batch buffer: store up to 32KB of log data before writing
    static constexpr size_t MAX_BATCH_BYTES = 32 * 1024;

    struct Line {
        uint16_t len;
        char data[512];
    };

    std::vector<Line> local_buffer;
    local_buffer.reserve(512);

    size_t batch_bytes = 0;

    while (running_ || !buffer_.empty() || !local_buffer.empty())
    {
        LogMessage msg;
        bool got_msg = buffer_.pop(msg);

        if (got_msg)
        {
            // --- Format timestamp in-place (no std::string!)
            char ts[32];
            size_t ts_len = format_timestamp(ts, msg.timestamp_);

            // --- Level as const char*
            const char* level_str = level_to_string(msg.level_);

            // --- Format a single line into fixed buffer
            Line& line = local_buffer.emplace_back();

            int n = std::snprintf(
                line.data,
                sizeof(line.data),
                "%.*s %s [T%u] %s\n",
                (int)ts_len, ts,
                level_str,
                msg.thread_id_,
                msg.message_
            );

            line.len = static_cast<uint16_t>(n);
            batch_bytes += n;
        }

        bool batch_full = batch_bytes >= MAX_BATCH_BYTES;

        // Write if full OR if shutting down
        if (!local_buffer.empty() && (batch_full || (!running_ && buffer_.empty())))
        {
            // Bulk append into a single big buffer
            std::string big;
            big.reserve(batch_bytes);

            for (auto& l : local_buffer)
                big.append(l.data, l.len);

            ::write(fd_, big.data(), big.size());

            local_buffer.clear();
            batch_bytes = 0;
        }

        // Block if no message
        if (!got_msg)
        {
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock, [&] {
                return !buffer_.empty() || !running_;
            });
        }
    }

    // Final flush
    if (!local_buffer.empty())
    {
        std::string big;
        size_t nbytes = 0;
        for (auto& l : local_buffer) nbytes += l.len;

        big.reserve(nbytes);
        for (auto& l : local_buffer)
            big.append(l.data, l.len);

        ::write(fd_, big.data(), big.size());
    }
}


