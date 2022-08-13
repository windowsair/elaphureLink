#pragma once

#include <cstdint>
#include <array>

typedef struct {
    uint32_t idcode;
    uint32_t irlen;
} jtag_idcode_info_t;

// TODO: C++20 to_array

enum {
    INVALID_IDCODE = 0
};

extern jtag_idcode_info_t k_jtag_idcode_list[];