//
// Created by Aris Lemmenes on 11/11/25.
//

#ifndef LFRBLOGGING_RINGBUFFER_H
#define LFRBLOGGING_RINGBUFFER_H

#include <iostream>

template<typename T, size_t capacity>
class RingBuffer
{
public:
    static_assert( ( capacity & (capacity-1) ) == 0 , "Ring Buffer has to have a size of a power of 2");
    bool push( const T& item );
    bool pop( T& item );
    [[nodiscard]] size_t get_size() const;
    [[nodiscard]] bool empty() const;
    void clear_all();
    [[nodiscard]] bool is_full() const;

    template<typename U, size_t N>
    friend std::ostream& operator<<(std::ostream& os, const RingBuffer<U, N>& rb);

private:
    T buffer_[capacity];
    size_t head_ { 0 };
    size_t tail_ { 0 };
};

#include "../src/RingBuffer.tpp"

#endif //LFRBLOGGING_RINGBUFFER_H