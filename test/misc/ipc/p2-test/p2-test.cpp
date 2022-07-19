#include <iostream>
#define ELL_DLL_USE_IMPORT 1
#include "../ipc_common.h"

#include <windows.h>

int main()
{
    int ret = ipc2_main_thread(1000 * 10000);
    if (ret == 0) {
        std::cout << "TEST PASS!\n";
    } else {
        std::cout << "TEST FAILED!\n";
    }
    return 0;

    int i = 0;
    // std::cout << "Hello World!\n";
}
