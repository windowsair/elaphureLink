#pragma once

#define PROXY_DLL_FUNCTION extern "C" __declspec(dllexport)

/*
 * Note: If not specified, all functions are thread unsafe.
 */



/**
 * @brief Initialize proxy resources, must call it at the beginning
 *
 * @return 0: on success, other on fail
 *
 */
PROXY_DLL_FUNCTION int el_proxy_init();
