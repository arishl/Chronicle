//
// Created by Aris Lemmenes on 11/13/25.
//

#include "AsyncBenchmarker.hpp"
#include <thread>
#include <iostream>
#include "../include/AsyncLogger/AsyncLogger.hpp"
#include "../include/AsyncLogger/LogLevel.hpp"

bool AsyncBenchmarker::benchmark_async()
{
    AsyncLogger logger("async_log.txt");

    logger.start();
    const LogLevel AUDIT {32, "[AUDIT]"};
    const LogLevel SQLITE {35, "[SQLITE]"};
    const auto start_async = std::chrono::high_resolution_clock::now();
    std::thread t1([&] {
        for (int i = 0; i < 50; ++i)
        {
            LogMessage msg(AUDIT, "Async log message", i);
            LogMessage msg1(LogLevel::TRACE, "Async log message", i);
            LogMessage msg2(SQLITE, "Async log message", i);
            for (int u = 0; u < 3000; u++)
            {
                logger.log(msg1);
                logger.log(msg);
                logger.log(msg2);
                logger.log(LogLevel::DEBUG, "FUCKED", i);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    t1.join();
    logger.stop();
    auto end_async = std::chrono::high_resolution_clock::now();
    auto async_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_async - start_async).count();
    std::cout << "Async logging took: " << async_ms << " ms\n";

    return true;
}

bool AsyncBenchmarker::benchmark_generic()
{
    auto start_sync = std::chrono::high_resolution_clock::now();
    std::thread t2([] {
        std::ofstream file("sync_log.txt");
        for (int i = 0; i < 50; ++i)
        {
            for (int u = 0; u < 3000; u++)
            {
                file << "Sync log message " << i << "\n";
                file.flush();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    t2.join();
    auto end_sync = std::chrono::high_resolution_clock::now();
    auto sync_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_sync - start_sync).count();
    std::cout << "Sync logging took: " << sync_ms << " ms\n";

    return true;
}

bool AsyncBenchmarker::benchmark_all()
{
    std::cout << "\n========== Logging Benchmarks ==========\n";
    benchmark_async();
    benchmark_generic();
    std::cout << "\n========================================\n";

    return true;
}

using namespace std::chrono;






