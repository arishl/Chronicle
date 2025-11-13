#include <thread>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "../include/AsyncLogger.h" // your class

int main()
{
    using namespace std::chrono;

    AsyncLogger logger("async_log.txt");
    logger.start();

    // --- Thread 1: Asynchronous logging ---
    auto start_async = high_resolution_clock::now();

    std::thread t1([&] {
        for (int i = 0; i < 50000; ++i)
        {
            LogMessage msg(LogLevel::INFO, "Async log message", i);
            logger.log(msg);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    t1.join();
    logger.stop();

    auto end_async = high_resolution_clock::now();
    auto async_ms = duration_cast<milliseconds>(end_async - start_async).count();

    std::cout << "Async logging took: " << async_ms << " ms\n";

    // --- Thread 2: Direct synchronous logging ---
    auto start_sync = high_resolution_clock::now();

    std::thread t2([] {
        std::ofstream file("sync_log.txt");
        for (int i = 0; i < 50000; ++i)
        {
            file << "Sync log message " << i << "\n";
            file.flush(); // simulate same flushing cost as frequent loggers
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    t2.join();

    auto end_sync = high_resolution_clock::now();
    auto sync_ms = duration_cast<milliseconds>(end_sync - start_sync).count();

    std::cout << "Sync logging took: " << sync_ms << " ms\n";

    return 0;
}