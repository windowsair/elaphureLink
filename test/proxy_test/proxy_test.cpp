#include "pch.h"
#include "CppUnitTest.h"

#define PROXY_DLL_USE_IMPORT 1
#include "../../common/proxy_export.hpp"

#include "windows.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace proxytest
{
TEST_CLASS (proxytest) {
    public:
    TEST_METHOD (handshake_test) {
        Assert::AreEqual(0,
                         el_proxy_init());
        Assert::AreEqual(0,
                         el_proxy_start_with_address("dap.local"));
        Sleep(INFINITE);
    }


    TEST_METHOD (stop_test) {
        Assert::AreEqual(0,
                         el_proxy_init());

        for (int i = 0; i < 10; i++) {
            Assert::AreEqual(0, el_proxy_start_with_address("dap.local"));
            el_proxy_stop();
        }
    }
};


} // namespace proxytest
