#pragma once

#include "pch.h"

#include <map>
#include <string>
#include <vector>


class ElaphureLinkRDDIContext
{
    public:
    ElaphureLinkRDDIContext()
    {
        debug_port_    = PORT_SWD;
        debug_clock_   = 1 * 1000 * 1000; // 1MHz
        is_master_     = true;
        is_SWJ_enable_ = true;

        is_swo_enable_ = false;
        swo_baudrate_  = 0;
    }

    using command_key_t   = std::string;
    using command_value_t = std::string;
    void setDebugConfigureFromList(std::vector<std::pair<command_key_t, command_value_t>> &commandList);

    enum DEBUG_PORT_MODE {
        PORT_SWD  = 0,
        PORT_JTAG = 1
    };

    private:
    // TODO:
    // memory handle

    private:
    // basic setting
    DEBUG_PORT_MODE debug_port_;
    int             debug_clock_;
    bool            is_SWJ_enable_;
    bool            is_master_;

    // swo setting
    bool        is_swo_enable_;
    int         swo_baudrate_;
    std::string swo_transport_;
};


extern ElaphureLinkRDDIContext kContext;
