#include <iostream>
#include "../ipc_common.h"


int main()
{
    std::cout << "Start to test p1\n";

    int ret = ipc1_main_thread(10 * 10000 + 1);
    if (ret == 0) {
        std::cout << "TEST PASS!\n";
    } else {
        std::cout << "TEST FAILED!\n";
    }

    return 0;
}
