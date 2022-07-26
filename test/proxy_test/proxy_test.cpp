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
    }
};
} // namespace proxytest
