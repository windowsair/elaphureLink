#include "pch.h"
#include "CppUnitTest.h"

#include "windows.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace unittest
{
TEST_CLASS (unittest) {
    public:
    TEST_METHOD (TestMethod1) {
        STARTUPINFO         si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));


        Assert::IsTrue(CreateProcess("p2-test.exe",
                                     NULL,  // Command line
                                     NULL,  // Process handle not inheritable
                                     NULL,  // Thread handle not inheritable
                                     FALSE, // Set handle inheritance to FALSE
                                     0,     // No creation flags
                                     NULL,  // Use parent's environment block
                                     NULL,  // Use parent's starting directory
                                     &si,   // Pointer to STARTUPINFO structure
                                     &pi));

        Sleep(1); // wait process to init

        BOOL ret = ipc_source_init();
        Assert::AreEqual(ret, TRUE, L"here 1");
        ret = ipc1_main_thread(100 * 10000);
        Assert::AreEqual(ret, 0, L"here 2");

        // cleanup
        TerminateProcess(pi.hProcess, 0);

    } // namespace unittest
};
} // namespace unittest
