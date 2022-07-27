#pragma once

enum DAPCommandEnum {
    ID_DAP_Info               = 0x00U,
    ID_DAP_HostStatus         = 0x01U,
    ID_DAP_Connect            = 0x02U,
    ID_DAP_Disconnect         = 0x03U,
    ID_DAP_TransferConfigure  = 0x04U,
    ID_DAP_Transfer           = 0x05U,
    ID_DAP_TransferBlock      = 0x06U,
    ID_DAP_TransferAbort      = 0x07U,
    ID_DAP_WriteABORT         = 0x08U,
    ID_DAP_Delay              = 0x09U,
    ID_DAP_ResetTarget        = 0x0AU,
    ID_DAP_SWJ_Pins           = 0x10U,
    ID_DAP_SWJ_Clock          = 0x11U,
    ID_DAP_SWJ_Sequence       = 0x12U,
    ID_DAP_SWD_Configure      = 0x13U,
    ID_DAP_SWD_Sequence       = 0x1DU,
    ID_DAP_JTAG_Sequence      = 0x14U,
    ID_DAP_JTAG_Configure     = 0x15U,
    ID_DAP_JTAG_IDCODE        = 0x16U,
    ID_DAP_SWO_Transport      = 0x17U,
    ID_DAP_SWO_Mode           = 0x18U,
    ID_DAP_SWO_Baudrate       = 0x19U,
    ID_DAP_SWO_Control        = 0x1AU,
    ID_DAP_SWO_Status         = 0x1BU,
    ID_DAP_SWO_ExtendedStatus = 0x1EU,
    ID_DAP_SWO_Data           = 0x1CU,
    ID_DAP_QueueCommands      = 0x7EU,
    ID_DAP_ExecuteCommands    = 0x7FU,
};

enum DAPResponseEnum {
    DAP_RES_OK     = 1,
    DAP_RES_WAIT   = 2,
    DAP_RES_FAULT  = 4,
    DAP_RES_NO_ACK = 7
};
