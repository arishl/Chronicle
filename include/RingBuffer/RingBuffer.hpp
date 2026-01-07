//
// Created by Aris Lemmenes on 11/11/25.
//

#ifndef LFRBLOGGING_RINGBUFFER_H
#define LFRBLOGGING_RINGBUFFER_H

#include <atomic>
#include <chrono>
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

    [[nodiscard]] std::optional<DataType> pop() noexcept;
    bool push(const DataType& aData) noexcept;
private:
    size_t mCapacity;
    size_t mMaxIndex;
    std::atomic<size_t> mRead;
    std::atomic<size_t> mWrite;
    std::atomic<size_t> mReservedWrite;
    DataType* mBuffer;
};

template<typename DataType>
RingBuffer<DataType>::RingBuffer(const size_t aCapacity)
    : mCapacity{aCapacity}, mMaxIndex {aCapacity-1} {
    mBuffer = new DataType[mCapacity];
}

template<typename DataType>
RingBuffer<DataType>::~RingBuffer() {
    delete[] mBuffer;
}

template <typename DataType>
std::optional<DataType> RingBuffer<DataType>::pop() noexcept {
    const auto cRead = mRead.load(std::memory_order_relaxed);
    const auto cWrite = mWrite.load(std::memory_order_relaxed);
    if (cRead == cWrite) {
        return std::nullopt;
    }
    return std::make_optional(mBuffer[cRead & mMaxIndex]);
}

template <typename DataType>
bool RingBuffer<DataType>::push(const DataType& aData) noexcept{
    while (true) {
        auto cReservedWrite = mReservedWrite.load(std::memory_order_relaxed);
        if (const auto cRead = mRead.load(std::memory_order_relaxed); cReservedWrite- cRead == mCapacity) {
            return false;
        }
        if (mReservedWrite.compare_exchange_weak(cReservedWrite, cReservedWrite+1, std::memory_order_relaxed)) {
            mBuffer[cReservedWrite & mMaxIndex] = aData;
            while (!mWrite.compare_exchange_weak(cReservedWrite, cReservedWrite+1, std::memory_order_release, std::memory_order_relaxed)) {
                std::this_thread::yield();
            }
            return true;
        }
    }
}


#endif //LFRBLOGGING_RINGBUFFER_H
