#include "pch.h"
#include "ElaphureLinkRDDIContext.h"

#include <cassert>
#include <stdexcept>
#include <functional>
#include <iostream>

#define SHOW_ERROR_MSG_BOX(msg) MessageBox(NULL, const_cast<LPCSTR>(msg), NULL, 0)


void ElaphureLinkRDDIContext::set_debug_configure_from_list(
    std::vector<std::pair<command_key_t, command_value_t>> &commandList)
{
    for (const auto &[key, value] : commandList) {
        set_debug_configure(key, value);
    }
}

void ElaphureLinkRDDIContext::set_debug_configure(const std::string &key, const std::string &value)
{
    if (key == "Master") {
        this->is_master_ = value == "Y";
    } else if (key == "Port") {
        this->debug_port_ = (value == "SW") ? PORT_SWD : PORT_JTAG;
    } else if (key == "Clock") {
        this->debug_clock_ = std::stoi(value);
    } else if (key == "SWJ") {
        // TODO:
    } else if (key == "SWJSwitch") {
        // switching from JTAG to SWD

        // See ARM Debug Interface Architecture Specification v5
        int sequence = std::stoul(value, nullptr, 16);
        assert(sequence == 0xE79E); // LSB sequence
    } else if (key == "JSWSwitch") {
        // switching from SWD to JTAG

        int sequence = std::stoul(value, nullptr, 16);
        assert(sequence == 0xE73C); // LSB sequence
    } else if (key == "Trace") {
        this->is_swo_enable_ = value != "Off";
    } else if (key == "TraceBaudrate") {
        this->swo_baudrate_ = std::stoi(value);
    } else if (key == "TraceTransport") {
        this->swo_transport_ = value;
    } else {
        SHOW_ERROR_MSG_BOX("unknown err");
    }
}
