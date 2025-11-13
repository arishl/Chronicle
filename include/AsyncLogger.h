//
// Created by Aris Lemmenes on 11/12/25.
//

#ifndef LFRBLOGGING_ASYNCLOGGER_H
#define LFRBLOGGING_ASYNCLOGGER_H
#include <cstdint>
#include <string>

enum class LogLevel
{
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,

};

struct LogMessage
{
    LogMessage(LogLevel level, char message[], uint32_t thread_id) :
        level_ {level}, message_ {message}, thread_id_ {thread_id}
    {

    }
    uint64_t timestamp_;
    uint32_t thread_id_;
    LogLevel level_;
    char message[256];
};

class AsyncLogger
{
public:
    explicit AsyncLogger(const std::string& filename);
    ~AsyncLogger();
    void log(LogMessage log_message);
    bool write_to_file();
};

#endif //LFRBLOGGING_ASYNCLOGGER_H