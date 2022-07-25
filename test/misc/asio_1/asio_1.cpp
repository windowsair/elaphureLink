#include <iostream>
#include <mutex>
#include <condition_variable>

#include <sdkddkver.h>
#include "thirdparty/asio/include/asio.hpp"
#include <windows.h>

using asio::ip::tcp;

using onSocketConnectCallbackType    = void (*)(const char *);
using onSocketDisconnectCallbackType = void (*)(const char *);

class SocketClient
{
    public:
    SocketClient()
        : is_running_(false),
          connect_callback_(nullptr),
          disconnect_callback_(nullptr)
    {
    }

    ~SocketClient()
    {
        // ~thread() require to join or detach
        if (main_thread_.joinable()) {
            main_thread_.detach();
        }
    }


    void send_something(void *buffer, size_t length)
    {
        asio::write(get_socket(), asio::buffer(buffer, length));
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
        asio::post(get_io_context(),
                   [this]() {
                       get_socket().close();
                   });
    }

    void do_connect(const tcp::resolver::results_type &endpoints)
    {
        asio::async_connect(get_socket(), endpoints,
                            [&](std::error_code ec, tcp::endpoint) {
                                is_running_post_done_ = true;

                                if (!ec) {
                                    // TODO: on connect

                                    asio::ip::tcp::no_delay option(true);
                                    get_socket().set_option(option);

                                    set_running_status(true, "Proxy started.");
                                    running_cv_.notify_all();
                                    do_read_header();
                                } else {
                                    set_running_status(false, ec.message());
                                    running_cv_.notify_all();
                                    close();
                                }
                            });
    }

    void do_read_header()
    {
        asio::ip::tcp::no_delay option;
        get_socket().get_option(option);
        bool is_set = option.value();
        assert(is_set == true);

        try {
            char data[10000];

            asio::error_code error;
            for (;;) {
                //Sleep(10 * 1000);

                //int ret = asio::read(socket_, asio::buffer(data), asio::transfer_exactly(1), error);

                size_t length = get_socket().read_some(asio::buffer(data), error);
                if (error == asio::error::eof) {
                    auto e = asio::system_error(error);
                    set_running_status(false, e.what());
                    break; // Connection closed cleanly by peer.
                } else if (error) {
                    throw asio::system_error(error); // Some other error.
                }


                asio::write(get_socket(), asio::buffer(data, length));

                std::cout.write(data, length);
            }
        } catch (std::exception &e) {
            set_running_status(false, e.what());
        }
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

class ProxyManager
{
    public:
    bool is_proxy_running()
    {
        if (client_.get()) {
            return client_.get()->is_socket_running();
        }

        return false;
    }

    void set_on_proxy_connect_callback(onSocketConnectCallbackType callback)
    {
        if (client_.get()) {
            client_.get()->set_connect_callback(callback);
        }
    }

    void set_on_proxy_disconnect_callback(onSocketDisconnectCallbackType callback)
    {
        if (client_.get()) {
            client_.get()->set_disconnect_callback(callback);
        }
    }

    int start_with_address(std::string address)
    {
        stop();
        client_ = std::make_unique<SocketClient>();


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
    std::unique_ptr<SocketClient> client_;
};

ProxyManager kManager;


extern "C" __declspec(dllexport) int start_proxy_with_address(char *address)
{
    return kManager.start_with_address(address);
}

extern "C" __declspec(dllexport) void stop_proxy()
{
    kManager.stop();
}


extern "C" __declspec(dllexport) void memory_leak_test()
{
    SocketClient c;

    int i = 0;
    while (i++ < 50) {
        c.init_socket("127.0.0.1", "88"); // use invalid address
        c.start();
        c.wait_main_thread();
    }
}

extern "C" __declspec(dllexport) int invalid_url_test(char *address, char *port)
{
    SocketClient c;

    int ret = c.init_socket(address, port);
    if (ret != 0) {
        return ret;
    }

    ret = c.start();
    Sleep(5 * 1000);
    c.kill();
    return ret;
}

void internal_invalid_url_test()
{
    char address[] = "www.bing.com";
    char port[]    = "80";

    invalid_url_test(const_cast<char *>("127.0.0.1"), const_cast<char *>("3240"));
    invalid_url_test(const_cast<char *>("www.bing.com"), const_cast<char *>("80"));
    invalid_url_test(const_cast<char *>("127.0.0.1"), const_cast<char *>("3240"));
    invalid_url_test(const_cast<char *>("127.0.0.1"), const_cast<char *>("80"));
    invalid_url_test(const_cast<char *>("1111111"), const_cast<char *>("3240"));
    invalid_url_test(const_cast<char *>("1.01.1.1"), const_cast<char *>("3240"));
    invalid_url_test(const_cast<char *>("1.0x01.1.1"), const_cast<char *>("3240"));

    Sleep(INFINITE);
}

extern "C" __declspec(dllexport) void tcp_no_delay_test()
{
    SocketClient c;

    char address[] = "127.0.0.1";
    char port[]    = "3240";

    int ret = c.init_socket(address, port);
    if (ret != 0) {
        throw "connect failed";
    }

    ret = c.start();
    if (ret != 0) {
        throw "connect failed";
    }

    char buffer[] = "123";
    try {
        c.send_something(buffer, 3);
        c.send_something(buffer, 3);
    } catch (std::exception &e) {
        std::string msg = e.what();
        throw msg;
    }

    Sleep(3000);
    return;
}


int main()
{
    return 0;
}