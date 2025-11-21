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
    template <class AllocatorType>
    void allocate(AllocatorType& aAllocator);
    bool is_allocated() const;
    template <class AllocatorType>
    void free(AllocatorType& aAllocator);
    template <typename... Args>
    bool emplace(Args&& ...aArgs);
    bool pop(Datatype& aPoppedData);
    bool empty() const noexcept;

private:
    static constexpr size_t sAlign = 64;
    alignas(sAlign) std::atomic<int> mPushIndex;
    alignas(sAlign) std::atomic<int> mPopIndex;
    alignas(sAlign) std::atomic<int> mSize;
    alignas(sAlign) std::byte* mStorage = nullptr;
    int mIndexEnd {0};

    int bump_index(int aIndex) const;
    void increase_size(int aNumPushed);
    void decrease_size(int aNumPopped);
};

template <typename DataType, WaitPolicy WaitPolicy, size_t Capacity>
template <class AllocatorType>
void RingBuffer<DataType, WaitPolicy, Capacity>::allocate(AllocatorType& aAllocator)
{
    if (is_allocated())
    {
        throw std::runtime_error("This memory has already been allocated");
    }
    if constexpr (Capacity == 0)
    {
        throw std::runtime_error("Capacity must be greater than zero");
    }

    auto cNumBytes = Capacity * sizeof(DataType);
    static constexpr auto sAlignment = std::max(sAlign, alignof(DataType));
    mStorage = static_cast<std::byte*>(aAllocator.allocate(cNumBytes, sAlignment));
    if (mStorage == nullptr)
    {
        throw std::runtime_error("Memory allocation failed!");
    }

    static constexpr int sMaxValue = std::numeric_limits<int>::max();
    static constexpr int sMaxWrap  = sMaxValue / Capacity;
    static_assert(sMaxWrap > 2, "Capacity too large: integer wrap protection insufficient");
    mIndexEnd = Capacity * sMaxWrap;
}

template <typename T, WaitPolicy WaitPolicy, size_t Capacity>
bool RingBuffer<T, WaitPolicy, Capacity>::is_allocated() const
{
    return mStorage != nullptr;
}

template <typename Datatype, WaitPolicy WaitPolicy, size_t Capacity>
template <class AllocatorType>
void RingBuffer<Datatype, WaitPolicy, Capacity>::free(AllocatorType& aAllocator)
{
    if (!is_allocated())
    {
        throw std::runtime_error("no memory to free...");
    }
    aAllocator.reset();
}

template <typename Datatype, WaitPolicy WaitPolicy, size_t Capacity>
template <typename ... ArgTypes>
bool RingBuffer<Datatype, WaitPolicy, Capacity>::emplace(ArgTypes&&... aArgs)
{
    const int cUnwrappedPushIndex {mPushIndex.load(std::memory_order::relaxed)};
    const int cUnwrappedPopIndex {mPopIndex.load(std::memory_order::acquire)};
    if (const int cIndexDelta = cUnwrappedPushIndex - cUnwrappedPopIndex; cIndexDelta == Capacity || cIndexDelta == Capacity - mIndexEnd)
    {
        return false;
    }
    const unsigned long cPushIndex = cUnwrappedPushIndex % Capacity;
    std::byte* cAddress = mStorage + cPushIndex * sizeof(Datatype);
    new (cAddress) Datatype(std::forward<ArgTypes>(aArgs)...);
    const auto cNewPushIndex = bump_index(cUnwrappedPushIndex);
    mPushIndex.store(cNewPushIndex, std::memory_order::release);
    increase_size(1);
    return true;
}

template <typename Datatype, WaitPolicy WaitPolicy, size_t Capacity>
bool RingBuffer<Datatype, WaitPolicy, Capacity>::pop(Datatype& aPoppedData)
{
    const int cUnwrappedPushIndex {mPushIndex.load(std::memory_order::relaxed)};
    const int cUnwrappedPopIndex {mPopIndex.load(std::memory_order::acquire)};
    if (cUnwrappedPushIndex == cUnwrappedPopIndex)
    {
        return false;
    }
    const unsigned long cPopIndex = cUnwrappedPopIndex % Capacity;
    std::byte* cAddress = mStorage + cPopIndex * sizeof(Datatype);
    Datatype* cData = std::launder(reinterpret_cast<Datatype*>(cAddress));
    aPoppedData = std::move(*cData);
    cData->~Datatype();
    const int cNewPopIndex = bump_index(cUnwrappedPopIndex);
    mPopIndex.store(cNewPopIndex, std::memory_order::release);
    decrease_size(1);
    return true;
}

template <typename Datatype, WaitPolicy WaitPolicy, size_t Capacity>
bool RingBuffer<Datatype, WaitPolicy, Capacity>::empty() const noexcept
{
    return mSize.load(std::memory_order::acquire) == 0;
}

template <typename Datatype, WaitPolicy WaitPolicy, size_t Capacity>
int RingBuffer<Datatype, WaitPolicy, Capacity>::bump_index(const int aIndex) const
{
    const int cIncrIndex = aIndex + 1;
    return cIncrIndex < mIndexEnd ? cIncrIndex : 0;
}

template <typename Datatype, WaitPolicy WaitPolicy, size_t Capacity>
void RingBuffer<Datatype, WaitPolicy, Capacity>::increase_size(const int aNumPushed)
{
    static constexpr auto sOrder = std::memory_order::relaxed;
    [[maybe_unused]] auto cPriorSize = mSize.fetch_add(aNumPushed, sOrder);
}

template <typename Datatype, WaitPolicy WaitPolicy, size_t Capacity>
void RingBuffer<Datatype, WaitPolicy, Capacity>::decrease_size(const int aNumPopped)
{
    static constexpr auto sOrder = std::memory_order::relaxed;
    [[maybe_unused]] auto cPriorSize = mSize.fetch_sub(aNumPopped, sOrder);
}

#endif //LFRBLOGGING_RINGBUFFER_H
