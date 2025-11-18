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

template <typename Datatype, WaitPolicy WaitPolicy, size_t Capacity>
class RingBuffer
{

public:
    template <class AllocationType>
    void Allocate(AllocationType& aAllocation);
    bool IsAllocated() const;
    template <class AllocationType>
    void Free(AllocationType& aAllocation);

private:
    static constexpr size_t sAlign = 64;

    alignas(sAlign) std::atomic<int> mPushIndex;
    alignas(sAlign) std::atomic<int> mPopIndex;
    alignas(sAlign) std::atomic<int> mSize;
    alignas(sAlign) std::byte* mStorage = nullptr;

    int mIndexEnd;
};

template <typename DataType, WaitPolicy WaitPolicy, size_t Capacity>
template <class AllocationType>
void RingBuffer<DataType, WaitPolicy, Capacity>::Allocate(AllocationType& aAllocation)
{
    if (IsAllocated())
    {
        throw std::runtime_error("This memory has already been allocated");
    }
    if constexpr (Capacity == 0)
    {
        throw std::runtime_error("Capacity must be greater than zero");
    }

    auto cNumBytes = Capacity * sizeof(DataType);
    static constexpr auto sAlignment = std::max(sAlign, alignof(DataType));
    mStorage = aAllocation.Allocate(cNumBytes, sAlignment);
    if (mStorage == nullptr)
    {
        throw std::runtime_error("Memory allocation failed!");
    }

    static constexpr int sMaxValue = std::numeric_limits<int>::max();
    static constexpr int cMaxWrap  = sMaxValue / Capacity;
    static_assert(cMaxWrap > 2, "Capacity too large: integer wrap protection insufficient");
    mIndexEnd = Capacity * cMaxWrap;
}

template <typename T, WaitPolicy WaitPolicy, size_t Capacity>
bool RingBuffer<T, WaitPolicy, Capacity>::IsAllocated() const
{
    return (mStorage != nullptr);
}

template <typename Datatype, WaitPolicy WaitPolicy, size_t Capacity>
template <class AllocationType>
void RingBuffer<Datatype, WaitPolicy, Capacity>::Free(AllocationType& aAllocation)
{

}

#endif //LFRBLOGGING_RINGBUFFER_H
