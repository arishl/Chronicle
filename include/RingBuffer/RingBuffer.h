//
// Created by Aris Lemmenes on 11/11/25.
//

#ifndef LFRBLOGGING_RINGBUFFER_H
#define LFRBLOGGING_RINGBUFFER_H

#include <atomic>
#include <chrono>
#include <cstddef>
#include <thread>
#include <new>



enum class ThreadsPolicy
{
    SPSC = 0,
    MPSC,
    SPMC,
    MPMC
};

enum class WaitPolicy
{
    NoWaits = 0,
    PushWait,
    PopWait,
    BothWait
};

template <typename T, size_t size, WaitPolicy WaitPolicy>
class RingBuffer
{

public:
    template <class AllocationType>
    void Allocate(AllocationType& allocation);
    bool IsAllocated() const;


private:
    static constexpr size_t sAlign = 64;

    alignas(sAlign) std::atomic<int> push_index_;
    alignas(sAlign) std::atomic<int> pop_index_;
    alignas(sAlign) std::atomic<int> size_;
    alignas(sAlign) std::byte*  storage_ = nullptr;

    int capacity_ = 0;
    int index_end_;
};

template <typename T, size_t size, WaitPolicy WaitPolicy>
template <class AllocationType>
void RingBuffer<T, size, WaitPolicy>::Allocate(AllocationType& allocation)
{


}

template <typename T, size_t size, WaitPolicy WaitPolicy>
bool RingBuffer<T, size, WaitPolicy>::IsAllocated() const
{

}

#endif //LFRBLOGGING_RINGBUFFER_H