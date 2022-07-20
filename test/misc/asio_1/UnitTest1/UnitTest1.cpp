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

namespace UnitTest1
{
TEST_CLASS (UnitTest1) {
    public:
    TEST_METHOD (reconnect_memory_leak) {
        CrtCheckMemory check;
        memory_leak_test();
    }
};
} // namespace UnitTest1
