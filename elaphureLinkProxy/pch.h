#pragma once

#define PROXY_EXPORT extern "C" __declspec(dllexport)

PROXY_EXPORT int test(int value);
