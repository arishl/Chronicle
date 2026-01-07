//
// Created by Aris Lemmenes on 11/11/25.
//

#ifndef LFRBLOGGING_RINGBUFFER_H
#define LFRBLOGGING_RINGBUFFER_H

#include <atomic>
#include <cstddef>
#include <thread>

template<typename DataType>
class RingBuffer {
public:
    explicit RingBuffer(size_t aCapacity);
    ~RingBuffer();
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer(RingBuffer&&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;
    RingBuffer& operator=(RingBuffer&&) = delete;

    [[nodiscard]] bool pop(DataType& aData) noexcept;
    bool push(const DataType& aData) noexcept;
    bool push(DataType&& data) noexcept;
    [[nodiscard]] bool is_empty() const noexcept;
private:
    static constexpr size_t CACHE_LINE {alignof(std::max_align_t)};
    size_t mCapacity;
    size_t mMaxIndex;
    alignas(CACHE_LINE) std::atomic<size_t> mRead;
    alignas(CACHE_LINE) std::atomic<size_t> mWrite;
    alignas(CACHE_LINE) std::atomic<size_t> mReservedWrite;
    DataType* mBuffer;
    template<typename U>
    bool push_impl(U&& aData) noexcept;
};

template<typename DataType>
RingBuffer<DataType>::RingBuffer(const size_t aCapacity)
    : mCapacity{aCapacity}, mMaxIndex {aCapacity-1} {
    mBuffer = new (std::align_val_t{CACHE_LINE}) DataType[mCapacity];
}

template<typename DataType>
RingBuffer<DataType>::~RingBuffer() {
    delete[] mBuffer;
}

template <typename DataType>
bool RingBuffer<DataType>::pop(DataType& aData) noexcept {
    const auto cRead {mRead.load(std::memory_order_relaxed)};
    const auto cWrite {mWrite.load(std::memory_order_acquire)};
    if (cRead == cWrite) {
        return false;
    }
    aData = mBuffer[cRead & mMaxIndex];
    mRead.store(cRead+1, std::memory_order_relaxed);
    return true;
}

template <typename DataType>
bool RingBuffer<DataType>::push(const DataType& aData) noexcept {
    return push_impl(aData);
}

template <typename DataType>
bool RingBuffer<DataType>::push(DataType&& data) noexcept {
    return push_impl(std::move(data));
}

template <typename DataType>
template <typename U>
bool RingBuffer<DataType>::push_impl(U&& aData) noexcept{
    const auto cWrite {mReservedWrite.fetch_add(1, std::memory_order_relaxed)};
    if (cWrite - mRead.load(std::memory_order_acquire) >= mCapacity) {
        mReservedWrite.fetch_sub(1, std::memory_order_relaxed);
        return false;
    }
    mBuffer[cWrite & mMaxIndex] = std::forward<U>(aData);
    auto cExpected {cWrite};
    while (!mWrite.compare_exchange_weak(
        cExpected,
        cWrite + 1,
        std::memory_order_release,
        std::memory_order_relaxed)) {
        cExpected = cWrite;
        std::atomic_signal_fence(std::memory_order_acq_rel);
    }
    return true;
}

template <typename DataType>
bool RingBuffer<DataType>::is_empty() const noexcept{
    return mRead.load(std::memory_order_acquire) == mWrite.load(std::memory_order_acquire);
}


#endif //LFRBLOGGING_RINGBUFFER_H
