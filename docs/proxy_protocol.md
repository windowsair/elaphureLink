# elaphureLink Proxy Protocol

The elaphureLink proxy protocol follows a server/client architecture. The server exports the DAP device and client imports it.

The elaphureLink proxy protocol is compliant with the CMSIS-DAP protocol. This means that any data transmitted in the elaphureLink proxy is a valid CMSIS-DAP command.

> For more information about CMSIS-DAP, see [CMSIS-DAP command](https://www.keil.com/pack/doc/CMSIS/DAP/html/group__DAP__Commands__gr.html)

There are two main phases in the elaphureLink Proxy Protocol, the handshake phase and the data transfer phase. All phases will be done in the same TCP/IP connection.

Due to the limitations that exist in the CMSIS-DAP protocol, all packet transmissions should be transmitted as a complete packet at the low level transport layer.

## Handshake phase

The client may ask the server if it supports the elaphureLink proxy protocol. To do this, the client opens a tcp/ip connection and sends a `REQ_HANDSHAKE` packet. Once the server supports the elaphureLink proxy protocol and receives a packet from the client, it will respond with `RES_HANDSHAKE` packet.

```
elaphureLink Proxy                                      DAP host
     "client"                                           "server"
 (imports DAP device)                             (exports DAP device)
         |                                                 |
         |                  REQ_HANDSHAKE                  |
         | ----------------------------------------------> |
         |                                                 |
         |                  RES_HANDSHAKE                  |
         | <---------------------------------------------- |
         |                                                 |
```

When the client receives `RES_HANDSHAKE`, it can decide by itself whether to proceed with the import of DAP devices. If the import of DAP devices is performed, then the data transfer phase can be started immediately, or it can be performed later. Otherwise, the client should disconnect the established TCP/IP connection.


`REQ_HANDSHAKE` and `RES_HANDSHAKE` are also valid CMSIS-DAP commands. More specifically, they are CMSIS-DAP Vendor Command, and such a convention simplifies the server implementation.


## Data transfer phase

During the data transmission phase, the CMSIS-DAP command is transmitted as is.

One or more packets can be transmitted in a single packet transmission. elaphureLink may prefer to merge multiple CMSIS-DAP commands and transmit them in the same packet. To this end, elaphureLink make the following conventions:

1. Use the `DAP_ExecuteCommands` to transfer multiple CMSIS-DAP commands in a single packet.
2. For client, if a CMSIS-DAP command with an unpredictable response length needs to be sent, the command is placed last and it can only appear once.


> For `DAP_ExecuteCommands`, see [DAP_ExecuteCommands command](https://www.keil.com/pack/doc/CMSIS/DAP/html/group__DAP__ExecuteCommands__gr.html)

The advantage of this is that for DAP host, few or no modifications are required to meet the requirements of the protocol.


```
elaphureLink Proxy                                      DAP host
     "client"                                           "server"
 (imports DAP device)                             (exports DAP device)
         |                                                 |
         |               REQ_CMSIS_DAP_COMMAND             |
         | ----------------------------------------------> |
         |                                                 |
         |               RES_CMSIS_DAP_COMMAND             |
         | <---------------------------------------------- |
         |                       .                         |
         |                       .                         |
         |                       .                         |
         |               REQ_CMSIS_DAP_COMMAND             |
         | ----------------------------------------------> |
         |                                                 |
         |               RES_CMSIS_DAP_COMMAND             |
         | <---------------------------------------------- |
         |                                                 |
```


----

## Packet Reference

The fields are in network (big endian) byte order meaning that the most significant byte (MSB) is stored at the lowest address.

**REQ_HANDSHAKE**

| Offset | Length | Value      | Description                            |
|--------|--------|------------|----------------------------------------|
| 0      | 4      | 0x8a656c70 | elaphureLink Proxy Protocol Identifier |
| 4      | 4      | 0x00000000 | Command code: handshake                |
| 8      | 4      |            | elaphureLink Proxy Version             |


**RES_HANDSHAKE**

| Offset | Length | Value      | Description                            |
|--------|--------|------------|----------------------------------------|
| 0      | 4      | 0x8a656c70 | elaphureLink Proxy Protocol Identifier |
| 4      | 4      | 0x00000000 | Command code: handshake                |
| 8      | 4      |            | elaphureLink DAP Firmware Version      |

