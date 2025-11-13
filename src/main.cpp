#include <thread>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "../include/AsyncLogger.h" // your class
#include "../benchmarks/AsyncBenchmarker.h"
int main()
{
    AsyncBenchmarker::benchmark_all();
    return 0;
}