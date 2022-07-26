#include "pch.h"

#include "SocketClient.hpp"
#include "protocol.hpp"


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
    for (;;) {
    }
}
