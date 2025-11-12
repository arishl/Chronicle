#include <iostream>
#include <string>

#include "../include/RingBuffer.h"

int main()
{
    std::string hold;
    RingBuffer<std::string, 4> rb;
    std::cout << rb << std::endl;
    rb.push("1");
    std::cout << rb << std::endl;
    rb.push("2");
    std::cout << rb << std::endl;
    rb.push("3");
    std::cout << rb << std::endl;
    rb.push("4");
    std::cout << rb << std::endl;
    rb.push("5");
    rb.pop(hold);

    std::cout << rb << std::endl;
    rb.clear_all();
    rb.pop(hold);
    std::cout << rb << std::endl;
    rb.pop(hold);
    std::cout << rb << std::endl;
    rb.pop(hold);
    std::cout << rb << std::endl;
    rb.pop(hold);
    std::cout << rb << std::endl;

    return 0;
}