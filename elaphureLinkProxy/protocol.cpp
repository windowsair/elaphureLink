#include "pch.h"

#include "SocketClient.hpp"
#include "protocol.hpp"


void SocketClient::set_keep_alive()
{
    if (k_windows_version_number.major_version < 10) {
        return;
    } else if (k_windows_version_number.major_version == 10 && k_windows_version_number.build_number < 16299) {
        // windows10 1709 build number:  16299
        return;
    }



    int ret;

    int enable_keepalive = 1;

    int keepalive_idle_time_secs       = 20;
    int keepalive_strobe_interval_secs = 5; // resend interval time
    int num_keepalive_strobes          = 5; // retry count

    ret = setsockopt(socket_.get()->native_handle(), SOL_SOCKET, SO_KEEPALIVE, (char *)&enable_keepalive, sizeof(enable_keepalive));
    if (ret != 0) {
        __debugbreak();
    }

    ret = setsockopt(socket_.get()->native_handle(), IPPROTO_TCP, TCP_KEEPCNT, (char *)&num_keepalive_strobes, sizeof(num_keepalive_strobes));
    if (ret != 0) {
        __debugbreak();
    }

    ret = setsockopt(socket_.get()->native_handle(), IPPROTO_TCP, TCP_KEEPIDLE, (char *)&keepalive_idle_time_secs, sizeof(keepalive_idle_time_secs));
    if (ret != 0) {
        __debugbreak();
    }

    ret = setsockopt(socket_.get()->native_handle(), IPPROTO_TCP, TCP_KEEPINTVL, (char *)&keepalive_strobe_interval_secs, sizeof(keepalive_strobe_interval_secs));
    if (ret != 0) {
        __debugbreak();
    }
}

void SocketClient::do_handshake()
{
    el_request_handshake_t req;
    req.el_link_identifier = htonl(EL_LINK_IDENTIFIER);
    req.command            = htonl(EL_COMMAND_HANDSHAKE);
    req.el_proxy_version   = htonl(EL_DAP_VERSION);

    asio::error_code ec;
    asio::write(get_socket(), asio::buffer(&req, sizeof(req)), ec);
    if (ec) {
        notify_connection_status(false, ec.message());
        close();
        return;
    }

    // get response
    el_response_handshake_t res;

    size_t n = asio::read(get_socket(),
                          asio::buffer(&res, sizeof(res)),
                          asio::transfer_exactly(sizeof(res)),
                          ec);
    if (ec) {
        notify_connection_status(false, ec.message());
        close();
        return;
    }

    if (ntohl(res.el_link_identifier) != EL_LINK_IDENTIFIER) {
        notify_connection_status(false, "connect failed: unexpected identifier");
        close();
        return;
    }

    if (ntohl(req.command) != EL_COMMAND_HANDSHAKE) {
        notify_connection_status(false, "connect failed: unexpected command");
        close();
        return;
    }

    return get_device_info();
}

void SocketClient::get_device_info()
{
    // get device info
    std::array<char, 1500> info_res_buffer;


    auto get_dap_info = [&](std::array<uint8_t, 2> buf) {
        asio::error_code ec;
        asio::write(get_socket(), asio::buffer(buf), asio::transfer_exactly(2), ec);
        if (ec) {
            notify_connection_status(false, ec.message());
            close();
            return false;
        }

        get_socket().read_some(asio::buffer(info_res_buffer), ec);
        //asio::read(get_socket(), asio::buffer(info_res_buffer), asio::transfer_all(), ec);
        if (ec) {
            notify_connection_status(false, ec.message());
            close();
            return false;
        }

        assert(info_res_buffer[0] == 0x00); // command
        assert(info_res_buffer[1] >= 1);    // len

        return true;
    };

    if (!get_dap_info({ 0x00, 0x02 })) { // Product Name
        return;
    }
    int len = info_res_buffer[1];
    memcpy(&(k_shared_memory_ptr->info_page.product_name), &info_res_buffer[2], len);



    if (!get_dap_info({ 0x00, 0x03 })) { // Serial Number
        return;
    }
    len = info_res_buffer[1];
    memcpy(&(k_shared_memory_ptr->info_page.serial_number), &info_res_buffer[2], len);

    if (!get_dap_info({ 0x00, 0x04 })) { // CMSIS-DAP Protocol Version(firmware version)
        return;
    }
    len = info_res_buffer[1];
    memcpy(&(k_shared_memory_ptr->info_page.firmware_version), &info_res_buffer[2], len);

    if (!get_dap_info({ 0x00, 0xF0 })) { // Capabilities
        return;
    }
    len = info_res_buffer[1];
    assert(len == 1 || len == 2);
    memcpy(&(k_shared_memory_ptr->info_page.capabilities), &info_res_buffer[2], len);


    // Ready to receive data of RDDI
    k_shared_memory_ptr->info_page.is_proxy_ready = 1;
    notify_connection_status(true, "connect succeeded");
    return do_data_process();
}

void SocketClient::do_data_process()
{
    asio::error_code          ec;
    std::array<uint8_t, 1500> res_buffer;
    int                       data_len;

    for (;;) {
        // step1: send request
        WaitForSingleObject(k_producer_event, INFINITE);
        if (!is_running_) {
            return; // socket close
        }

        asio::write(get_socket(),
                    asio::buffer(&(k_shared_memory_ptr->producer_page.data), k_shared_memory_ptr->producer_page.data_len),
                    ec);
        if (ec) {
            set_running_status(false, ec.message());
            close();
            return;
        }

        // step2: receive response
        data_len = get_socket().read_some(asio::buffer(res_buffer), ec);
        if (ec) {
            set_running_status(false, ec.message());
            close();
            return;
        }

        //// FIXME: verify response data length

        // step3: parse response
        uint8_t *p        = res_buffer.data();
        int      count    = *p == ID_DAP_ExecuteCommands ? *(p + 1) : 1;
        bool     out_flag = false;

        if (*p == ID_DAP_ExecuteCommands) { // skip header
            p += 2;
        }

        for (; count > 0; count--) {
            switch (*p) {
                case ID_DAP_Connect: {
                    p += 2;
                    break;
                }
                case ID_DAP_TransferConfigure: {
                    p += 2;
                    set_consumer_status(DAP_RES_OK); // FIXME: check response status?
                    break;
                }
                case ID_DAP_Transfer: {
                    int transfer_count = (int)*++p;
                    int status         = (int)*++p;
                    p++; // point to data

                    if (transfer_count != k_shared_memory_ptr->producer_page.command_count) {
                        out_flag = true;

                        set_consumer_status(DAP_RES_FAULT);
                        break;
                    }

                    set_consumer_status(status);
                    if (status != DAP_RES_OK) {
                        // not OK
                        out_flag = true;
                        break;
                    }

                    int remain_data_len = data_len - (p - res_buffer.data());
                    assert(remain_data_len % 4 == 0); // FIXME: close and clean up
                    k_shared_memory_ptr->consumer_page.data_len = remain_data_len;
                    memcpy(k_shared_memory_ptr->consumer_page.data, p, remain_data_len);

                    break;
                }

                case ID_DAP_TransferBlock: {
                    p++;
                    const int transfer_count = ((*(p + 1)) << 8) | (*p);
                    p += 2;

                    const int status = *p++;


                    if (transfer_count != k_shared_memory_ptr->producer_page.command_count) {
                        // FIXME:
                        out_flag = true;

                        set_consumer_status(DAP_RES_FAULT);
                        break;
                    }

                    set_consumer_status(status);
                    if (status != DAP_RES_OK && status != DAP_RES_FAULT) {
                        // not OK
                        out_flag = true;
                        break;
                    }

                    const int remain_data_len = data_len - (p - res_buffer.data());
                    assert(remain_data_len % 4 == 0); // FIXME:
                    k_shared_memory_ptr->consumer_page.data_len = remain_data_len;
                    memcpy(k_shared_memory_ptr->consumer_page.data, p, remain_data_len);

                    break;
                }

                case ID_DAP_WriteABORT: {
                    if (*(p + 1) != 0) { // status code
                        set_consumer_status(DAP_RES_FAULT);
                        out_flag = true;
                    } else {
                        set_consumer_status(DAP_RES_OK);
                    }

                    break;
                }

                case ID_DAP_SWJ_Clock: {
                    p += 2;
                    break;
                }
                case ID_DAP_SWJ_Sequence: {
                    p += 2;
                    break;
                }
                case ID_DAP_SWD_Configure: {
                    p += 2;
                    break;
                }
                default:
                    close();
                    return;
            }

            if (out_flag)
                break;
        }

        if (out_flag) {
            // set out_command invalid
        }




        // step4: notify
        SetEvent(k_consumer_event);
    }
}
