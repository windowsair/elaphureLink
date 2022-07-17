#include "pch.h"


#include <iostream>


PROXY_EXPORT int test(int value)
{
    std::cout << "input: " << value << std::endl;
    return value;
}