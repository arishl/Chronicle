//
// Created by Aris Lemmenes on 11/11/25.
//

template <typename T, size_t size, ThreadsPolicy Threading,
    WaitPolicy Waiting>
bool RingBuffer<T, size, Threading, Waiting>::push(const T& item)
{
    const size_t next_head { (head_ + 1) & (size - 1) };
    if (next_head == tail_)
    {
        return false;
    }
    buffer_[head_] = item;
    head_ = next_head;

    return true;
}

template <typename T, size_t size, ThreadsPolicy Threading,
    WaitPolicy Waiting>
bool RingBuffer<T, size, Threading, Waiting>::pop(T& item)
{
    if (tail_ == head_){
        return false;
    }
    item = buffer_[tail_];
    tail_ = (tail_ + 1) & (size - 1);
    return true;
}

template <typename T, size_t size, ThreadsPolicy Threading,
    WaitPolicy Waiting>
size_t RingBuffer<T, size, Threading, Waiting>::get_size() const
{
    if (head_ >= tail_)
    {
        return head_ - tail_;
    }
    return size + head_ - tail_;
}

template <typename T, size_t size, ThreadsPolicy Threading,
    WaitPolicy Waiting>
bool RingBuffer<T, size, Threading, Waiting>::empty() const
{
    return head_ == tail_;
}

template <typename T, size_t size, ThreadsPolicy Threading,
    WaitPolicy Waiting>
void RingBuffer<T, size, Threading, Waiting>::clear_all()
{
    head_ = tail_ = 0;
}

template <typename T, size_t size, ThreadsPolicy Threading,
    WaitPolicy Waiting>
bool RingBuffer<T, size, Threading, Waiting>::is_full() const
{
    return ((head_ + 1) & (size-1)) == tail_;
}

template <typename U, size_t N, ThreadsPolicy X,
    WaitPolicy V>
std::ostream& operator<<(std::ostream& os, const RingBuffer<U, N, X, V>& rb)
{
    os << "RingBuffer(size=" << N << ", elements=" << rb.get_size() << "): [";
    size_t i = rb.tail_m;
    bool first = true;
    while (i != rb.head_m) {
        if (!first) os << ", ";
        os << rb.buffer_m[i];
        i = (i + 1) & (N - 1);
        first = false;
    }
    os << "]";
    return os;
}
