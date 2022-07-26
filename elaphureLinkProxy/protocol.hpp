#pragma once
#include <cstdint>

#define EL_LINK_IDENTIFIER   0x8a656c70

#define EL_DAP_VERSION       0x00000001

#define EL_COMMAND_HANDSHAKE 0x00000000


typedef struct
{
    uint32_t el_link_identifier;
    uint32_t command;
    uint32_t el_proxy_version;
} el_request_handshake_t;


typedef struct
{
    uint32_t el_link_identifier;
    uint32_t command;
    uint32_t el_dap_version;
} el_response_handshake_t;