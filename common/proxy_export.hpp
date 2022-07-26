#pragma once

#ifndef PROXY_DLL_USE_IMPORT
#define PROXY_DLL_FUNCTION extern "C" __declspec(dllexport)
#else
#define PROXY_DLL_FUNCTION extern "C" __declspec(dllimport)
#endif


/*
 * Note: If not specified, all functions are thread unsafe.
 */

using onSocketConnectCallbackType    = void (*)(const char *);
using onSocketDisconnectCallbackType = void (*)(const char *);



/**
 * @brief Initialize proxy resources, must call it at the beginning
 *
 * @return 0: on success, other on fail
 *
 */
PROXY_DLL_FUNCTION int el_proxy_init();


/**
 * @brief Start the Proxy with the specified address.
 *
 * @param address DAP host url address
 * @return 0: on success, other on fail
 */
PROXY_DLL_FUNCTION int el_proxy_start_with_address(char *address);


/**
 * @brief Force the Proxy to stop. This function can be used at any time.
 *
 */
PROXY_DLL_FUNCTION void el_proxy_stop();


/**
 * @brief Set the callback function to be used when the Proxy connection is successfully established.
 *        This function should be called before `el_proxy_start_with_address`
 *
 * @param callback
 */
PROXY_DLL_FUNCTION void el_proxy_set_on_connect_callback(onSocketConnectCallbackType callback);


/**
 * @brief Set the callback function to be used when the Proxy connection is disconnected.
 *
 * @param callback
 */
PROXY_DLL_FUNCTION void el_proxy_set_on_disconnect_callback(onSocketDisconnectCallbackType callback);
