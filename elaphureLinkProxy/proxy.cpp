#include "pch.h"

#include "SocketClient.hpp"


class ProxyManager
{
    public:
    ProxyManager()
        : on_connect_callback_(nullptr),
          on_socket_disconnect_callback_(nullptr)
    {
    }

    bool is_proxy_running()
    {
        if (client_.get()) {
            return client_.get()->is_socket_running();
        }

        return false;
    }

    void set_on_proxy_connect_callback(onSocketConnectCallbackType callback)
    {
        on_connect_callback_ = callback;

        if (client_.get()) {
            client_.get()->set_connect_callback(callback);
        }
    }

    void set_on_proxy_disconnect_callback(onSocketDisconnectCallbackType callback)
    {
        on_socket_disconnect_callback_ = callback;

        if (client_.get()) {
            client_.get()->set_disconnect_callback(callback);
        }
    }

    int start_with_address(std::string address)
    {
        stop();
        client_ = std::make_unique<SocketClient>();

        if (on_connect_callback_) {
            client_.get()->set_connect_callback(on_connect_callback_);
        }
        if (on_socket_disconnect_callback_) {
            client_.get()->set_disconnect_callback(on_socket_disconnect_callback_);
        }

        int ret = client_.get()->init_socket(address, "3240");
        if (ret != 0) {
            return ret;
        }

        return client_.get()->start();
    }

    void stop()
    {
        if (client_.get()) {
            client_.get()->kill();
        }
        client_.reset(nullptr);
    }

    private:
    onSocketConnectCallbackType    on_connect_callback_;
    onSocketDisconnectCallbackType on_socket_disconnect_callback_;
    std::unique_ptr<SocketClient>  client_;
};

ProxyManager k_manager;


PROXY_DLL_FUNCTION int el_proxy_start_with_address(char *address)
{
    return k_manager.start_with_address(address);
}


PROXY_DLL_FUNCTION void el_proxy_stop()
{
    return k_manager.stop();
}


PROXY_DLL_FUNCTION void el_proxy_set_on_connect_callback(onSocketConnectCallbackType callback)
{
    return k_manager.set_on_proxy_connect_callback(callback);
}


PROXY_DLL_FUNCTION void el_proxy_set_on_disconnect_callback(onSocketDisconnectCallbackType callback)
{
    return k_manager.set_on_proxy_disconnect_callback(callback);
}
