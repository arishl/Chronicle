//
// Created by Aris Lemmenes on 11/17/25.
//

#ifndef LFRBLOGGING_ALLOCATOR_H
#define LFRBLOGGING_ALLOCATOR_H

class Allocator
{
public:
    explicit Allocator(int aBufferSize);
    ~Allocator();
    Allocator(const Allocator&) = delete;
    Allocator (const Allocator&&) = delete;
    void* allocate(int aAllocationSize, int aAlignment);
    void reset();
private:
    void* mBufferOffestPtr {nullptr};
    void* mBufferStartPtr {nullptr};
    unsigned long mTotalSize;
    unsigned long mUsedAmount {0};
};



#endif //LFRBLOGGING_ALLOCATOR_H
