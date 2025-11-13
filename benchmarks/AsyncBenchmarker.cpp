//
// Created by Aris Lemmenes on 11/13/25.
//

#include "AsyncBenchmarker.h"
#include <thread>
#include "../include/AsyncLogger.h"

bool AsyncBenchmarker::benchmark_async()
{
    AsyncLogger logger("async_log.txt");
    logger.start();
    auto start_async = std::chrono::high_resolution_clock::now();
    std::thread t1([&] {
        for (int i = 0; i < 50; ++i)
        {
            LogMessage msg(LogLevel::INFO, "Async log message", i);
            for (int u = 0; u < 3000; u++)
            {
                logger.log(msg);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    t1.join();
    logger.stop();
    auto end_async = std::chrono::high_resolution_clock::now();
    auto async_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_async - start_async).count();
    std::cout << "Async logging took: " << async_ms << " ms\n";
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
}

bool AsyncBenchmarker::benchmark_all()
{
    std::cout << "\n========== Logging Benchmarks ==========\n";
    benchmark_async();
    benchmark_generic();
    std::cout << "\n========================================\n";
}

using namespace std::chrono;






