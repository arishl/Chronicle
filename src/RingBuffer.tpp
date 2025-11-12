//
// Created by Aris Lemmenes on 11/11/25.
//

template <typename T, size_t size>
bool RingBuffer<T, size>::push(const T& item)
{
    const size_t next_head = (head_m + 1) & (size - 1);
    if (next_head == tail_m)
    {
        return false;
    }
    buffer_m[head_m] = item;
    head_m = next_head;

    return true;
}

template <typename T, size_t size>
bool RingBuffer<T,size>::pop(T& item)
{
    if (tail_m == head_m){
        return false;
    }
    item = buffer_m[tail_m];
    tail_m = (tail_m + 1) & (size - 1);

    return true;
}

template <typename T, size_t size>
size_t RingBuffer<T, size>::get_size() const
{
    if (head_m >= tail_m)
    {
        return head_m - tail_m;
    }
    return size + head_m - tail_m;
}

template <typename T, size_t size>
bool RingBuffer<T, size>::empty() const
{
    return head_m == tail_m;
}

template <typename T, size_t size>
void RingBuffer<T, size>::clear_all()
{
    head_m = tail_m = 0;
}

template <typename T, size_t size>
bool RingBuffer<T, size>::is_full() const
{
    return ((head_m + 1) & (size-1)) == tail_m;
}

template <typename U, size_t N>
std::ostream& operator<<(std::ostream& os, const RingBuffer<U, N>& rb)
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
