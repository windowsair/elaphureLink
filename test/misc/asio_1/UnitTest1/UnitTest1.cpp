#include "pch.h"
#include "CppUnitTest.h"
#include <crtdbg.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

struct CrtCheckMemory {
    _CrtMemState state1;
    _CrtMemState state2;
    _CrtMemState state3;
    CrtCheckMemory()
    {
        _CrtMemCheckpoint(&state1);
    }
    ~CrtCheckMemory()
    {
        _CrtMemCheckpoint(&state2);
        Assert::AreEqual(0, _CrtMemDifference(&state3, &state1, &state2), L"Opps, memory leak!");
        // else just do this to dump the leaked blocks to stdout.
        if (_CrtMemDifference(&state3, &state1, &state2))
            _CrtMemDumpStatistics(&state3);
    }
};


extern "C" __declspec(dllimport) void memory_leak_test();
extern "C" __declspec(dllimport) int invalid_url_test(char *address, char *port);
extern "C" __declspec(dllimport) void tcp_no_delay_test();

extern "C" __declspec(dllimport) int start_proxy_with_address(char *address);
extern "C" __declspec(dllimport) void stop_proxy();



namespace UnitTest1
{
TEST_CLASS (UnitTest1) {
    public:
    TEST_METHOD (reconnect_memory_leak) {
        CrtCheckMemory check;
        memory_leak_test();
    }

    TEST_METHOD (url_test) {
        // Various scenarios of connection breakage will also be implicitly tested.
        Assert::AreEqual(0, invalid_url_test("www.bing.com", "80"));
        Assert::AreEqual(0, invalid_url_test("127.0.0.1", "3240"));

        Assert::AreNotEqual(0, invalid_url_test("1.1.1.1:80", "3240"));
        Assert::AreNotEqual(0, invalid_url_test("1.0x01.1.1", "3240")); // hex
        Assert::AreNotEqual(0, invalid_url_test("1.01.1.1", "3240"));   // octal
    }

    TEST_METHOD (tcp_no_delay) {
        tcp_no_delay_test();
    }

    TEST_METHOD (manager_1) {
        Assert::AreEqual(0,
                         start_proxy_with_address("127.0.0.1"));

        stop_proxy();

        Assert::AreEqual(0,
                         start_proxy_with_address("127.0.0.1"));

        stop_proxy();

        //
        Assert::AreEqual(0,
                         start_proxy_with_address("127.0.0.1"));
        Assert::AreEqual(0,
                         start_proxy_with_address("127.0.0.1"));
        Assert::AreEqual(0,
                         start_proxy_with_address("127.0.0.1"));
    }

    TEST_METHOD (manager_2) {
        Assert::AreEqual(0,
                         start_proxy_with_address("127.0.0.1"));
        Assert::AreEqual(0,
                         start_proxy_with_address("127.0.0.1"));
        Assert::AreEqual(0,
                         start_proxy_with_address("127.0.0.1"));
    }

    TEST_METHOD (manager_3) {
        stop_proxy();
        stop_proxy();
        stop_proxy();

        Assert::AreEqual(0,
                         start_proxy_with_address("127.0.0.1"));
        Assert::AreEqual(0,
                         start_proxy_with_address("127.0.0.1"));
        Assert::AreEqual(0,
                         start_proxy_with_address("127.0.0.1"));
    }

    TEST_METHOD (manager_4) {
        Assert::AreNotEqual(0,
                            start_proxy_with_address("127.0.0.2"));
        Assert::AreEqual(0,
                         start_proxy_with_address("127.0.0.1"));
        Assert::AreNotEqual(0,
                            start_proxy_with_address("127.0.0.2"));
        Assert::AreEqual(0,
                         start_proxy_with_address("127.0.0.1"));
    }
};
} // namespace UnitTest1
