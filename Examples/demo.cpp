#include "../benchmarks/AsyncBenchmarker.hpp"

#include "../include/RingBuffer/RingBuffer.hpp"
#include "../include/Allocator/Allocator.hpp"
#include <iostream>
/**
int main()
{
    AsyncBenchmarker::benchmark_all();
    return 0;
}
**/

int main()
{
    // 1. Create an allocator (backing storage for ring buffer)
    Allocator allocator(1024 * 1024); // 1MB backing buffer

    // 2. Create a ring buffer: type int, no waiting, capacity 8
    RingBuffer<int, WaitPolicy::NoWaits, 8> ring;

    // 3. Allocate internal storage using our allocator
    ring.allocate(allocator);

    // 4. Push values
    for (int i = 1; i <= 5; ++i)
    {
        if (ring.emplace(i))
            std::cout << "Pushed: " << i << "\n";
        else
            std::cout << "Push failed (full)\n";
    }

    // 5. Pop values
    while (!ring.empty())
    {
        int value;
        if (ring.pop(value))
        {
            std::cout << "Popped: " << value << "\n";
        }
    }

    // 6. Free the allocator
    ring.free(allocator);

    return 0;
}