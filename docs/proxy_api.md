# elaphureLink Proxy Document

elaphureLinkProxy provides a set of API for managing the establishment and disconnection of connections, as well as callback functions for when connection establishment and disconnection events occur.

If not specified, all functions are **thread unsafe**.

All exported functions use the **cdelc** calling convention.

## Example

The following simple example shows how to connect to the Proxy and set the callback function when connection disconnect.


```c
#include <stdio.h>
#include <windows.h>

void on_proxy_disconnect_callback(const char* msg)
{
    printf("proxy exit. Reason: %s\n", msg);

    exit(0);
}

int start_proxy(char* proxy_address)
{
    int ret;
    ret = el_proxy_init();
    if (ret != 0) {
        printf("Could not start proxy. Exit.\n");
        return ret; // failed
    }

    ret = el_proxy_start_with_address(proxy_address);
    if (ret != 0) {
        return ret; // failed
    }

    el_proxy_set_on_disconnect_callback(on_proxy_disconnect_callback);

    return 0;
}


int main()
{
    int ret;
    ret = start_proxy("dap.local");
    if (ret = 0) {
        exit(1); // failed
    }

    Sleep(INFINITE); // wait proxy thread to exit.

    return 0;
}

```

## API Reference


### `el_proxy_init`

Initialize proxy resources, must call it at the beginning.

```c
int el_proxy_init();
```

return 0 on success, other on fail.


### `el_proxy_start_with_address`

Start the Proxy with the specified address.

```c
int el_proxy_start_with_address(char *address);
```

return 0 on success, other on fail.


### `el_proxy_stop`

Force the Proxy to stop. This function can be used at any time.

```c
void el_proxy_stop();
```


### `el_proxy_set_on_connect_callback`

Set the callback function to be used when the Proxy connection is successfully established.

```c
void el_proxy_set_on_connect_callback(onSocketConnectCallbackType callback);
```


### `el_proxy_set_on_disconnect_callback`

Set the callback function to be used when the Proxy connection is disconnected.

```c
void el_proxy_set_on_disconnect_callback(onSocketDisconnectCallbackType callback);
```


### `onSocketConnectCallbackType`

```c
typedef void (*onSocketConnectCallbackType)(const char* msg);
```

`msg` is a string message provided by the proxy to the callback function about the reason for the establishment.


### `onSocketDisconnectCallbackType`

```c
typedef void (*onSocketDisconnectCallbackType)(const char* msg);
```

`msg` is a string message provided by the proxy to the callback function about the reason for the disconnection.