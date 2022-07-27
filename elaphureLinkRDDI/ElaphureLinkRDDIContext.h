#pragma once

#include "pch.h"

#include <cstdint>
#include <string>
#include <vector>


class ElaphureLinkRDDIContext
{
    public:
    ElaphureLinkRDDIContext()
    {
        rddi_handle_ = -1; // invalid handle

        debug_port_    = PORT_SWD;
        debug_clock_   = 1 * 1000 * 1000; // 1MHz
        is_master_     = true;
        is_SWJ_enable_ = true;

        is_swo_enable_ = false;
        swo_baudrate_  = 0;
    }


    using command_key_t   = std::string;
    using command_value_t = std::string;
    void set_debug_configure(const std::string &key, const std::string &value);
    void set_debug_configure_from_list(std::vector<std::pair<command_key_t, command_value_t>> &commandList);

    enum RDDI_HANDLE_ENUM {
        INVALID_RDDI_HANDLE_NUM = -1,
    };

    void set_rddi_handle(RDDIHandle value)
    {
        rddi_handle_ = value;
    }

    RDDIHandle get_rddi_handle()
    {
        // FIXME: ?
        if (!k_shared_memory_ptr->info_page.is_proxy_ready) {
            rddi_handle_ = INVALID_RDDI_HANDLE_NUM;
        }
        return rddi_handle_;
    }

    int get_debug_clock()
    {
        return debug_clock_;
    }

    std::vector<uint32_t>& get_dap_list()
    {
        return dap_list_;
    }

    enum DEBUG_PORT_MODE {
        PORT_SWD  = 0,
        PORT_JTAG = 1
    };

    private:
    // TODO:
    // memory handle

    private:
    RDDIHandle rddi_handle_;
    // basic setting
    DEBUG_PORT_MODE debug_port_;
    int             debug_clock_;
    bool            is_SWJ_enable_;
    bool            is_master_;

    // swo setting
    bool        is_swo_enable_;
    int         swo_baudrate_;
    std::string swo_transport_;

    // info
    std::vector<uint32_t> dap_list_;
};


extern ElaphureLinkRDDIContext kContext;
