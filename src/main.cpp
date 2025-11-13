#include <iostream>
#include <string>

#include "../include/RingBuffer.h"
#include "../include/AsyncLogger.h"
#include <thread>
int main()
{
    AsyncLogger logger("app.log");
    logger.start();

    std::thread t1([&]{
        for (int i = 0; i < 50; ++i)
        {
            LogMessage msg(LogLevel::INFO, "Thread 1 logging", i);

            logger.log(msg);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    t1.join();

    logger.stop();
    return 0;
}