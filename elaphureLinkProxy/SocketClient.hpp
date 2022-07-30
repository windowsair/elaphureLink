#pragma once
#include <iostream>
#include <mutex>
#include <condition_variable>

#include <sdkddkver.h>
#include "thirdparty/asio/include/asio.hpp"

#include "pch.h"

using asio::ip::tcp;

class SocketClient
{
    public:
    SocketClient()
        : is_running_(false),
          connect_callback_(nullptr),
          disconnect_callback_(nullptr)
    {
    }

    void do_something();

    ~SocketClient()
    {
        // FIXME: access violation

        // ~thread() require to join or detach
        if (main_thread_.joinable()) {
            main_thread_.join();
        }
    }

    int init_socket(std::string address, std::string port = "3240")
    {
        socket_.reset(nullptr); // prevent double free

        io_context_ = std::make_unique<asio::io_context>();
        tcp::resolver resolver(get_io_context());

        socket_ = std::make_unique<asio::ip::tcp::socket>(get_io_context());

        try {
            endpoint_ = resolver.resolve(address, port);
        } catch (std::exception &e) {
            return -1;
        }

        return 0;
    }


    void kill()
    {
        is_running_ = false;
        if (k_is_proxy_init) {
			k_shared_memory_ptr->info_page.is_proxy_ready = 0;
            k_shared_memory_ptr->consumer_page.command_response = 0xFFFFFFFF; // invalid value
            SetEvent(k_producer_event);                                       // wake up
        }

        socket_.get()->close();
        io_context_.get()->stop();
        // Resources should not be released immediately, as this will result in a deadlock.
        Sleep(100);
    }

    int start()
    {
        using namespace std::chrono_literals;

        std::unique_lock<std::mutex> lk(running_status_mutex_);
        if (is_running_) {
            return 0; // already start
        }

        is_running_post_done_ = false;
        is_running_           = true;

        do_connect(endpoint_);

        main_thread_ = std::thread([&]() { // start main thread
            try {
                get_io_context().run();
            } catch (std::exception &e) {
                set_running_status(false, e.what());
            }

        });

        running_cv_.wait(lk, [this]() { return is_running_post_done_; });

        return !is_running_;
    }

    void wait_main_thread()
    {
        main_thread_.join();
    }


    //
    //
    // setter/getter
    public:
    asio::io_context &get_io_context()
    {
        return *(io_context_.get());
    }

    asio::ip::tcp::socket &get_socket()
    {
        return *(socket_.get());
    }

    bool is_socket_running()
    {
        return is_running_;
    }

    void set_running_status(bool status, const std::string msg)
    {
        is_running_ = status;
        if (status) {
            if (connect_callback_) {
                connect_callback_(msg.c_str());
            }
        } else {
            if (disconnect_callback_) {
                disconnect_callback_(msg.c_str());
            }
        }
    }

    void set_connect_callback(onSocketConnectCallbackType callback)
    {
        connect_callback_ = callback;
    }

    void set_disconnect_callback(onSocketDisconnectCallbackType callback)
    {
        disconnect_callback_ = callback;
    }

    private:
    void close()
    {
        is_running_ = false;
        if (k_is_proxy_init) {
            k_shared_memory_ptr->info_page.is_proxy_ready = 0;
			k_shared_memory_ptr->consumer_page.command_response = 0xFFFFFFFF; // invalid value
            SetEvent(k_producer_event);                                       // wake up
        }

        asio::post(get_io_context(),
                   [this]() {
                       get_socket().close();
                   });
    }

    void do_connect(const tcp::resolver::results_type &endpoints)
    {
        asio::async_connect(get_socket(), endpoints,
                            [&](std::error_code ec, tcp::endpoint) {
                                //is_running_post_done_ = true;
                                if (!ec) {
                                    asio::ip::tcp::no_delay option(true);
                                    get_socket().set_option(option);

                                    do_handshake();
                                } else {
                                    notify_connection_status(false, ec.message());
                                    close();
                                }
                            });
    }

    // handshake phase
    void do_handshake();

    // data phase
    void get_device_info();
    void do_data_process();

    void notify_connection_status(bool status, const std::string msg)
    {
        is_running_post_done_ = true;
        set_running_status(status, msg);
        running_cv_.notify_all();
    }


    private:
    bool                    is_running_;
    bool                    is_running_post_done_;
    std::mutex              running_status_mutex_;
    std::condition_variable running_cv_;

    std::unique_ptr<asio::io_context> io_context_;
    std::unique_ptr<tcp::socket>      socket_;

    tcp::resolver::results_type endpoint_;

    std::thread main_thread_;

    onSocketConnectCallbackType    connect_callback_;
    onSocketDisconnectCallbackType disconnect_callback_;
};
