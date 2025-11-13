//
// Created by Aris Lemmenes on 11/12/25.
//
#include <fstream>
#include <iostream>

#include "../include/AsyncLogger.h"


AsyncLogger::AsyncLogger(const std::string& filename) :
    file_(filename, std::ios::app)
{
    if (!file_)
    {
        throw std::runtime_error("Could not open file");
    }
    file_ << std::flush;
}

AsyncLogger::~AsyncLogger()
{
    stop();
    if (file_.is_open())
        file_.close();
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

void AsyncLogger::worker_loop()
{
    using namespace std::chrono;

    std::vector<std::string> local_buffer;
    local_buffer.reserve(512);

    auto last_flush = steady_clock::now();

    while (running_ || !buffer_.empty() || !local_buffer.empty())
    {
        LogMessage msg;
        bool got_msg = buffer_.pop(msg);

        if (got_msg)
        {
            const std::string timestamp = AsyncLogger::format_timestamp(msg.timestamp_);
            const std::string level_str = AsyncLogger::level_to_string(msg.level_);

            std::ostringstream oss;
            oss << timestamp << " " << level_str
                << " [T" << msg.thread_id_ << "] "
                << msg.message_ << "\n";

            local_buffer.push_back(std::move(oss).str());
        }

        auto now = steady_clock::now();
        bool time_to_flush = duration_cast<milliseconds>(now - last_flush).count() >= 10000;
        bool batch_full = local_buffer.size() >= 512;

        if (!local_buffer.empty() && (time_to_flush || batch_full || (!running_ && buffer_.empty())))
        {
            for (auto& line : local_buffer)
                file_ << line;

            file_.flush();
            local_buffer.clear();
            last_flush = now;
        }


        if (!got_msg)
        {
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait_for(lock, std::chrono::milliseconds(1), [&] {
                return !buffer_.empty() || !running_;
            });
        }

    }

    // Final flush in case anything is left
    for (auto& line : local_buffer)
        file_ << line;
    file_.flush();
}

std::string AsyncLogger::level_to_string(const LogLevel level)
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

std::string AsyncLogger::format_timestamp(const uint64_t timestamp_ms)
{
    using namespace std::chrono;
    const auto tp = time_point<system_clock, milliseconds>(milliseconds(timestamp_ms));
    const std::time_t t = system_clock::to_time_t(tp);
    const std::tm tm = *std::localtime(&t);

    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);

    const auto ms_part = timestamp_ms % 1000;

    std::ostringstream oss;
    oss << buf << "." << std::setfill('0') << std::setw(3) << ms_part;
    return oss.str();
}
