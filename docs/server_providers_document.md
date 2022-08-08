# elaphureLink Server Providers Document

During each connection, a specific protocol needs to be followed between elaphureLinkProxy and debug unit. For more information about the protocol, see [proxy_protocol](proxy_protocol.md).


In addition, the following requirements need to be met for service providers:

1. elaphureLink will ignore the **Packet Count** and **Packet Size** information in the DAP. As an alternative, you should make sure to reserve a contiguous buffer larger than `1400` byte for each packet.

2. You should make sure that the **product name** of the DAP device contains the substring "CMSIS-DAP". Otherwise, the device will not be recognized correctly.



## Example

In the following example, we will show how to build a simple elaphureLink service provider with a TCP server and the CMSIS-DAP function.

```c

#define BUF_SIZE 1500
uint8_t el_process_buffer[BUF_SIZE];


#define EL_LINK_IDENTIFIER 0x8a656c70
#define EL_DAP_VERSION 0x00000001
#define EL_COMMAND_HANDSHAKE 0x00000000

typedef struct
{
    uint32_t el_link_identifier;
    uint32_t command;
    uint32_t el_proxy_version
} el_request_handshake_t;

typedef struct
{
    uint32_t el_link_identifier;
    uint32_t command;
    uint32_t el_dap_version
} el_response_handshake_t;


int elaphureLink_handshake(int fd, void *buffer, size_t len)
{
    // verify

    if (len != sizeof(el_request_handshake)) {
        return -1;
    }

    el_request_handshake_t* req = (el_request_handshake*)buffer;

    if (ntohl(req->el_link_identifier) != EL_LINK_IDENTIFIER) {
        return -1;
    }

    if (ntohl(req->command) != EL_COMMAND_HANDSHAKE) {
        return -1;
    }

    // send response

    el_response_handshake_t res;
    res.el_link_identifier = htonl(EL_LINK_IDENTIFIER);
    res.command = htonl(EL_COMMAND_HANDSHAKE);
    res.el_dap_version = htonl(EL_DAP_VERSION);

    send(fd, &res, sizeof(el_response_handshake_t), 0);

    return 0;
}


void elaphureLink_process(int fd, void *buffer, size_t len)
{
    int length = DAP_ExecuteCommand(buffer, (uint8_t *)el_process_buffer);
    length &= 0xFFFF; // get response length

    send(fd, el_process_buffer, length, 0);
}


void tcp_server()
{
    uint8_t buf[BUF_SIZE];
    int socketfd;
    size_t length;
    int status = 0;

    do_bind(&socketfd);
    do_accept(&socketfd);

    for (;;) {
        length = recv(socketfd, buf, BUF_SIZE, 0);
        if (length <= 0) {
            break;
        }

        switch (status) {
            case 0:
                if (elaphureLink_handshake(socketfd, buf, length) != 0) {
                    break;
                }
                status = 1;
                break;
            case 1:
                elaphureLink_process(socketfd, buf, length);
        }
    }

    close(socketfd);
}
```

