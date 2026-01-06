//
// Created by Aris Lemmenes on 11/17/25.
//
#include "../../include/Allocator/Allocator.hpp"


#include <memory>
#include <cstdlib>

Allocator::Allocator(int const aBufferSize) : mTotalSize{
    static_cast<unsigned long>(aBufferSize)
} {
    mBufferStartPtr = std::malloc(mTotalSize);
    mBufferOffestPtr = mBufferStartPtr;
}

Allocator::~Allocator() {
    free(mBufferStartPtr);
}

void* Allocator::allocate(const int aAllocationSize, const int aAlignment) {
    std::size_t remainingSize{mTotalSize - mUsedAmount};
    void* cAlignedPointer{
        std::align(aAlignment, aAllocationSize, mBufferOffestPtr, remainingSize)
    };
    if (cAlignedPointer != nullptr) {
        const unsigned long cAlignmentSize = reinterpret_cast<std::uintptr_t>(
            cAlignedPointer) - reinterpret_cast<std::uintptr_t>(
            mBufferOffestPtr);
        mUsedAmount += cAlignmentSize + aAllocationSize;
        mBufferOffestPtr = reinterpret_cast<void*>(aAllocationSize +
            reinterpret_cast<uintptr_t>(cAlignedPointer));
    }
    return cAlignedPointer;
}

void Allocator::reset() {
    std::free(mBufferStartPtr);
    mBufferStartPtr = std::malloc(mTotalSize);
    mBufferOffestPtr = mBufferStartPtr;
    mUsedAmount = 0;
}
