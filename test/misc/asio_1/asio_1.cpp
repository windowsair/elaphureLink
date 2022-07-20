#include <iostream>
#include <mutex>
#include <condition_variable>
#include <sdkddkver.h>
#include "thirdparty/asio/include/asio.hpp"
#include <windows.h>

using asio::ip::tcp;

class socketClient
{
    public:
    socketClient()
        : isRunning_(false)
    {
    }

    void init_socket(std::string address)
    {
        socket_.reset(nullptr); // prevent double free

        io_context_ = std::make_unique<asio::io_context>();
        tcp::resolver resolver(get_io_context());

        socket_ = std::make_unique<asio::ip::tcp::socket>(get_io_context());

        endpoint_ = resolver.resolve(address, "3240");
    }

    asio::io_context &get_io_context()
    {
        return *(io_context_.get());
    }

    asio::ip::tcp::socket &get_socket()
    {
        return *(socket_.get());
    }

    int start()
    {
        using namespace std::chrono_literals;

        std::unique_lock<std::mutex> lk(running_status_mutex_);
        if (isRunning_) {
            return 0; // already start
        }

        isRunning_post_done_ = false;
        isRunning_           = true;

        do_connect(endpoint_);

        main_thread_ = std::thread([&]() { // start main thread
            get_io_context().run();
        });

        running_cv_.wait(lk, [this]() { return isRunning_post_done_; });

        return !isRunning_;
    }

    void wait_main_thread()
    {
        main_thread_.join();
    }

    void close()
    {
        isRunning_ = false;
        asio::post(get_io_context(),
                   [this]() { get_socket().close(); });
    }

    private:
    void do_connect(const tcp::resolver::results_type &endpoints)
    {
        asio::async_connect(get_socket(), endpoints,
                            [&](std::error_code ec, tcp::endpoint) {
                                isRunning_post_done_ = true;

                                if (!ec) {
                                    isRunning_ = true;
                                    running_cv_.notify_all();
                                    do_read_header();
                                } else {
                                    isRunning_ = false;
                                    running_cv_.notify_all();
                                    std::cout << ec.message();
                                    close();
                                }
                            });
    }

    void do_read_header()
    {
        try {
            char data[10000];

            asio::error_code error;
            for (;;) {
                //Sleep(10 * 1000);

                //int ret = asio::read(socket_, asio::buffer(data), asio::transfer_exactly(1), error);

                size_t length = get_socket().read_some(asio::buffer(data), error);
                if (error == asio::error::eof) {
                    auto        x   = asio::system_error(error);
                    std::string msg = x.what();
                    isRunning_      = false;
                    break; // Connection closed cleanly by peer.
                } else if (error) {
                    isRunning_ = false;
                    throw asio::system_error(error); // Some other error.
                }


                asio::write(get_socket(), asio::buffer(data, length));

                std::cout.write(data, length);
            }
        } catch (std::exception &e) {
            isRunning_ = false;
            std::cerr << "Exception in thread: " << e.what() << "\n";
        }
    }


    private:
    bool                    isRunning_;
    bool                    isRunning_post_done_;
    std::mutex              running_status_mutex_;
    std::condition_variable running_cv_;

    std::unique_ptr<asio::io_context> io_context_;
    std::unique_ptr<tcp::socket>      socket_;

    tcp::resolver::results_type endpoint_;

    std::thread main_thread_;
};

extern "C" __declspec(dllexport) void memory_leak_test()
{
    socketClient c;
    int          i = 0;
    while (i++ < 50) {
        c.init_socket("127.0.0.1");
        c.start();
        c.wait_main_thread();
    }
}

//int main()
//{
//    memory_leak_test();
//
//    return 0;
//}
