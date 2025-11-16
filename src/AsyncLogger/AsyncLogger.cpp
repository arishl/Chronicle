//
// Created by Aris Lemmenes on 11/12/25.
//
#include <fstream>
#include <unistd.h>
#include <fcntl.h>

#include "../../include/AsyncLogger/AsyncLogger.h"


AsyncLogger::AsyncLogger(const std::string& filename) : buffer_()
{
    fd_ = ::open(filename.c_str(),
                 O_WRONLY | O_CREAT | O_APPEND,
                 0644);

    if (fd_ < 0)
    {
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

bool AsyncLogger::log(const LogMessage& log_message)
{
    const bool pushed = buffer_.push(log_message);
    if (pushed)
        cv_.notify_one();
    return pushed;
}

bool AsyncLogger::log(const LogLevel level, const char* message, const uint32_t thread_id)
{
    const LogMessage msg(level, message, thread_id);
    const bool pushed = log(msg);
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

size_t AsyncLogger::format_timestamp(char* out, uint64_t timestamp_ms)
{
    using namespace std::chrono;

    const auto tp = time_point<system_clock, milliseconds>(milliseconds(timestamp_ms));
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
        static_cast<unsigned long long>(timestamp_ms % 1000)
    );

    return static_cast<size_t>(n);
}

void AsyncLogger::worker_final_check(std::vector<Line>& local_buffer) const
{
    if (!local_buffer.empty())
    {
        std::string big;
        size_t capacity = 0;
        for (auto& [len, data] : local_buffer)
        {
            capacity += len;
        }
        big.reserve(capacity);
        for (auto& [len, data] : local_buffer)
        {
            big.append(data, len);
        }
        ::write(fd_, big.data(), big.size());
    }
}

void AsyncLogger::worker_loop()
{
    static constexpr size_t MAX_BATCH_BYTES = 32 * 1024;
    std::vector<Line> local_buffer;
    local_buffer.reserve(512);
    size_t batch_bytes = 0;
    while (running_ || !buffer_.empty() || !local_buffer.empty())
    {
        LogMessage msg {};
        const bool got_msg = buffer_.pop(msg);
        if (got_msg)
        {
            char ts[32];
            const size_t ts_len = format_timestamp(ts, msg.timestamp_);
            const char* level_str = LogLevel::to_string(msg.level_);
            const char* level_color = LogLevel::color_of(msg.level_);
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
                msg.message_
            );
            line.len = static_cast<uint16_t>(n);
            batch_bytes += n;
        }
        if (const bool batch_full = batch_bytes >= MAX_BATCH_BYTES; !local_buffer.empty() && (batch_full || (!running_ && buffer_.empty())))
        {
            std::string big;
            big.reserve(batch_bytes);
            for (auto& l : local_buffer)
            {
                big.append(l.data, l.len);
            }
            ::write(fd_, big.data(), big.size());
            local_buffer.clear();
            batch_bytes = 0;
        }
        if (!got_msg)
        {
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock, [&] {
                return !buffer_.empty() || !running_;
            });
        }
    }

    worker_final_check(local_buffer);

}


