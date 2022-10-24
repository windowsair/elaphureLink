/**************************************************************************/ /**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.1.13
 * @date     $Date: 2019-07-02 15:48:51 +0200 (Tue, 02 Jul 2019) $
 *
 * @note
 * Copyright (C) 2009-2019 ARM Limited. All rights reserved.
 *
 * @brief     Low Level Layer for the SWD (Serial Wire) Interface
 *
 * @par
 * ARM Limited (ARM) is supplying this software for use with Keil uVision
 * and Cortex-M processor based microcontrollers.
 *
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

/*
SWD_ReadD32/D16/D8: reads a single 32/16/8-bit data
SWD_ReadBlock: reads 32-bit elements inside R/W page (up to max RWBlock)
SWD_ReadBlockD8/D16/D32: read block of memory with 32/16/8-bit accesses only
SWD_ReadARMMem: reads any memory (size, alignment)
SWD_ReadARMMemD8/D16/D32: reads memory with 32/16/8-bit accesses regardless of alignment

Different function are there for optimization access speed on low level if the interface supports it. If driver supports only generic read function then the D32/D16/D8 and Block function can be implemented to use this generic function. All function need to be implemented.

SWD_SysCallExec: Sets the relevant core registers and starts it
SWD_SysCallRes: Returns the result of the SysCall (register R0).
*/



/*  Usage of AP_Context
 *  ===================
 *
 *  AP_Context stores information for the system's MEM-APs and basic information about how
 *  to program the MEM-AP's CSW register. A context is selected by a pair of DP and AP indexes:
 *  - DP index: The index of the DP in the chain. There is exactly one DP index '0' for SWD.
 *              The indexing for JTAG is from the JTAG-TAP nearest to TDO to the JTAG-TAP nearest to TDI.
 *
 *              TDO <- Custom JTAG-TAP <- CoreSight JTAG-DP <- ... <- CoreSight JTAG-DP <- TDI
 *                        (Index n)          (Index n-1)                 (Index 0)
 *
 *  - AP index: AP index behind selected DP as programmed into DP SELECT.
 *
 *  DP/AP selection
 *  ---------------
 *
 *  The following global variables store the selection info parameters:
 *  - nCPU             : Index of selected DP for debugged CPU.
 *  - MonConf.AP       : Index of AP selected for debugged CPU.
 *
 *  - JTAG_devs.com_no : Index of currently selected DP. Can be different from nCPU if executing
 *                       a debug sequence or when accessing a global debug/trace component behind
 *                       a different DP than the debugged CPU.
 *  - AP_Sel           : Stores the APSEL field value as used in DP SELECT register to access
 *                       the currently selected AP. The represented AP index can be different
 *                       from MonConf.AP if executing a debug sequence or when accessing a global
 *                       debug/trace component behind a different DP/AP than the debugged CPU.
 *                       NOTE: Format differes from MonConf.AP. Example when selecting the AP
 *                       for the debugged CPU:
 *                                              AP_Sel = (MonConf.AP << 24);
 *
 *  Setting up target memory accesses based on AP_Context
 *  -----------------------------------------------------
 *
 *  1. Get the AP context via
 *
 *       int AP_Switch(AP_CONTEXT** apCtx);
 *
 *     It gets the currently selected context (JTAG_devs.com_no, AP_Sel) via the function AP_CurrentCtx().
 *     If the MEM-AP is accessed for the first time, the AP IDR register is analyzed and initial context
 *     values are determined.
 *
 *  2. Check if the requested access size is supported in AP_CONTEXT::AccSizes.
 *
 *  3. SPROT bit in AP_CONTEXT::CSW_Val_Base should be updated based on access attributes via _UpdateAPSecAttr()
 *     function (JTAG.cpp and SWD.cpp). See the function implementation for more details.
 *
 *  4. The value that shall be written to the selected CSW register of the MEM-AP shall be assembled by a
 *     bit-wise OR operation of the following components:
 *     - AP_CONTEXT::CSW_Val_Base - SPROT is expected to be updated in this member in previous step.
 *     - SIZE field as for requested access size (indicated by access function name)
 *     - AddrInc field
 *        - Off, if attribute BLOCK_NADDRINC is set for access.
 *        - Increment Single, if attribute BLOCK_NADDRINC is NOT set and if AP_CONTEXT:PT is '0'.
 *        - Increment Packed, if attribute BLOCK_NADDRINC is NOT set and if AP_CONTEXT:PT is '1'.
 *
 */


#include "stdafx.h"
#include "..\AGDI.h"
#include "Collect.h"
#include "Debug.h"
#include "JTAG.h"
#include "SWD.h"
#include "..\BOM.h"
#include "rddi_dll.hpp"

#if DBGCM_DBG_DESCRIPTION
#include "PDSCDebug.h"
#endif // DBGCM_DBG_DESCRIPTION

#if DBGCM_DS_MONITOR
#include "DSMonitor.h"
#endif // DBGCM_DS_MONITOR
#include <cassert>
#include <mutex>

DWORD SWD_IDCode; // SWD ID Code

static std::recursive_mutex kSWDOpMutex;

#if DBGCM_V8M
// Forward declarations
static int SWD_UpdateDSCSR(DWORD adr, DWORD many, BYTE attrib);
#endif // DBGCM_V8M


// SWJ Reset
//   return value: error status
int SWJ_Reset(void)
{
    // RDDI will handle this automatically, no need to send the sequence again
    return (0);
}


// SWJ Switch: SWD <-> JTAG
//   val    : Switch Code (16-bit)
//   return value: error status
int SWJ_Switch(WORD val)
{
    std::lock_guard<std::recursive_mutex> lk(kSWDOpMutex);
    //...
    DEVELOP_MSG("Todo: \nImplement Function: int SWJ_Switch (WORD val)");
    return (0);
}


// SWD Read ID Code
//   return value: error status
int SWD_ReadID(void)
{
    int noOfDAPs;
    int idcode;

    if (rddi::CMSIS_DAP_DetectNumberOfDAPs(rddi::k_rddi_handle, &noOfDAPs) != RDDI_SUCCESS) {
        return EU10;
    }

    rddi::CMSIS_DAP_DetectDAPIDList(rddi::k_rddi_handle, &idcode, 1);
    SWD_IDCode = idcode;

    switch (SWD_IDCode & 0x0FF1FFFF) {
        case 0x0BA01477: // DP V1
        case 0x0BA02477: // DP V2
        case 0x0BA11477: // DP V1 MinDP
        case 0x0BA12477: // DP V2 MinDP
        case 0x0BB11477: // DP V1 MinDP (ARM Cortex-M0)
        case 0x0BC11477: // DP V1 MinDP (ARM Cortex-M0+)
        case 0x0BD11477: // DP V1 MinDP (ARM Cortex-M7)
        case 0x0BC12477: // DP V2 MinDP (ARM Cortex-M0+ with Multi-Drop SW)
        case 0x0BD12477: // DP V2 MinDP (ARM Cortex-M7 with Multi-Drop SW)
        case 0x0BE12477: // DP V2 MinDP (ARM Cortex-M33)
        case 0x0BF11477: // DP V1 MinDP (ARM Cortex-M23)
        case 0x0BF12477: // DP V2 MinDP (ARM Cortex-M23 with Multi-Drop SW)
            break;
        default:
            return (EU10);
    }

    JTAG_devs.cnt          = 1;
    JTAG_devs.ic[0].id     = SWD_IDCode;
    JTAG_devs.ic[0].ir_len = 0;
    strcpy(JTAG_devs.icname[0], "ARM CoreSight SW-DP");

    return (0);
}


int SWD_CheckStatus(int status)
{
    if (status == RDDI_DAP_OPERATION_TIMEOUT) {
        status = SWD_DAPAbortVal(DAPABORT);
        if (status)
            return (status);
        return (rddi::RDDI_DAP_ERROR_MEMORY);
    }
    if (status == RDDI_DAP_DP_STICKY_ERR) {
        // Only for SW (not availalbe for JTAG)
        status = SWD_DAPAbortVal(STKERRCLR | WDERRCLR);
        if (status)
            return (status);
        return (rddi::RDDI_DAP_ERROR_MEMORY);
    }
    if (status)
        return (rddi::RDDI_DAP_ERROR);

    return (0);
}

static int SWD_CheckStickyError(DWORD dp_stat)
{
    int status;

    if (dp_stat & (STICKYERR | WDATAERR)) {
        status = SWD_DAPAbortVal(STKERRCLR | WDERRCLR);
        if (status)
            return (status);
        return (rddi::RDDI_DAP_ERROR_MEMORY);
    }

    return (0);
}

static int SWD_StickyError(void)
{
    int   status;
    DWORD dp_stat;

    status = SWD_ReadDP(DP_CTRL_STAT, &dp_stat);
    if (status)
        return (status);

    status = SWD_CheckStickyError(dp_stat);
    return (status);
}


//   adr    : Address
//   val    : Pointer to Value
//   return : 0 - Success, else Error Code
static int SWD_ReadData(DWORD adr, DWORD *val)
{
    int status;
    int regID[2];
    int regData[2];

    // TAR = adr
    regID[0]   = DAP_AP_REG_TAR;
    regData[0] = adr;

    // DRW read
    regID[1] = DAP_AP_REG_DRW | DAP_REG_RnW;

    // R/W DAP Registers
    status = rddi::DAP_RegAccessBlock(rddi::k_rddi_handle, 0, 2, regID, regData);
    status = SWD_CheckStatus(status);
    if (status)
        return (status);

    *val = regData[1];

    return (0);
}


//   adr    : Address
//   val    : Value
//   return : 0 - Success, else Error Code
static int SWD_WriteData(DWORD adr, DWORD val)
{
    int status;
    int regID[2];
    int regData[2];

    // TAR = adr
    regID[0]   = DAP_AP_REG_TAR;
    regData[0] = adr;

    // DRW = val
    regID[1]   = DAP_AP_REG_DRW;
    regData[1] = val;

    // R/W DAP Registers
    status = rddi::DAP_RegAccessBlock(rddi::k_rddi_handle, 0, 2, regID, regData);
    status = SWD_CheckStatus(status);
    if (status)
        return (status);

    return (0);
}

// SWD Data/Access Port Abort
//   return value: error status
int SWD_DAPAbort(void)
{
    //...
    DEVELOP_MSG("Todo: \nImplement Function: int SWD_DAPAbort (void)");
    return (0);
}


// SWD Read DP Register
//   adr    : Address
//   val    : Pointer to Value
//   return value: error status
int SWD_ReadDP(BYTE adr, DWORD *val)
{
    std::lock_guard<std::recursive_mutex> lk(kSWDOpMutex);

#if DBGCM_DBG_DESCRIPTION
    int status = 0, pstatus = 0, retry_count = 1;
#endif // DBGCM_DBG_DESCRIPTION

    // Read DP Register
    do {
        status = rddi::DAP_ReadReg(rddi::k_rddi_handle, 0,
                                   DAP_REG_DP_0x0 + (adr >> 2), (int *)val);
        status = SWD_CheckStatus(status);
        if (status == rddi::RDDI_DAP_ERROR_MEMORY) {
            if (retry_count == 0)
                return EU01;
        } else {
            break;
        }
    } while (retry_count-- > 0);

    if (status == rddi::RDDI_DAP_ERROR_MEMORY) {
        return EU14;
    } else if (status != 0) {
        return EU01;
    }


#if DBGCM_DBG_DESCRIPTION
    if (PDSCDebug_IsEnabled()) {
        pstatus = PDSCDebug_PatchData((U32)ACCESS_DP, 4, adr, 4, (UC8 *)val, 0 /*attrib*/);
        if (pstatus == 0) { // Error handling for data patches...
            status = 0;     // ... if data patched then clear previous access error.
        } else if (pstatus != EU38 && status == 0) {
            status = pstatus; // ... if error during patch and no previous error.
        }
        if (status)
            return (status);
    }
#endif // DBGCM_DBG_DESCRIPTION

    return (0);
}


// SWD Write DP Register
//   adr    : Address
//   val    : Value
//   return value: error status
int SWD_WriteDP(BYTE adr, DWORD val)
{
    std::lock_guard<std::recursive_mutex> lk(kSWDOpMutex);

    int status = 0, retry_count = 1;

    // Read DP Register
    do {
        status = rddi::DAP_WriteReg(rddi::k_rddi_handle, 0,
                                    DAP_REG_DP_0x0 + (adr >> 2), val);
        status = SWD_CheckStatus(status);
        if (status == rddi::RDDI_DAP_ERROR_MEMORY) {
            if (retry_count == 0)
                return EU01;
        } else {
            break;
        }
    } while (retry_count-- > 0);

    if (status == rddi::RDDI_DAP_ERROR_MEMORY) {
        return EU14;
    } else if (status != 0) {
        return EU01;
    }

    return (0);
}


// SWD Read AP Register
//   adr    : Address
//   val    : Pointer to Value
//   return value: error status
int SWD_ReadAP(BYTE adr, DWORD *val)
{
    std::lock_guard<std::recursive_mutex> lk(kSWDOpMutex);

#if DBGCM_DBG_DESCRIPTION
    int status = 0, pstatus = 0;
#endif // DBGCM_DBG_DESCRIPTION

    if ((adr ^ AP_Bank) & APBANKSEL) {
        status = SWD_WriteDP(DP_SELECT, AP_Sel | (adr & APBANKSEL));
        if (status)
            return (status);
        AP_Bank = adr & APBANKSEL;
    }

    adr &= 0x0F;

    // Read AP Register
    status = rddi::DAP_ReadReg(rddi::k_rddi_handle, 0,
                               DAP_REG_AP_0x0 + (adr >> 2), (int *)val);
    status = SWD_CheckStatus(status);
    if (status == rddi::RDDI_DAP_ERROR_MEMORY) {
        return EU14;
    } else if (status != 0) {
        return EU01;
    }

#if DBGCM_DBG_DESCRIPTION
    if (PDSCDebug_IsEnabled()) {
        pstatus = PDSCDebug_PatchData((U32)ACCESS_AP, 4, adr, 4, (UC8 *)val, 0 /*attrib*/);
        if (pstatus == 0) { // Error handling for data patches...
            status = 0;     // ... if data patched then clear previous access error.
        } else if (pstatus != EU38 && status == 0) {
            status = pstatus; // ... if error during patch and no previous error.
        }
        if (status)
            return (status);
    }
#endif // DBGCM_DBG_DESCRIPTION

    return (0);
}


// SWD Write AP Register
//   adr    : Address
//   val    : Value
//   return value: error status
int SWD_WriteAP(BYTE adr, DWORD val)
{
    std::lock_guard<std::recursive_mutex> lk(kSWDOpMutex);

    int status;

    if ((adr ^ AP_Bank) & APBANKSEL) {
        status = SWD_WriteDP(DP_SELECT, AP_Sel | (adr & APBANKSEL));
        if (status)
            return (status);
        AP_Bank = adr & APBANKSEL;
    }

    adr &= 0x0F;

    // Write AP Register
    status = rddi::DAP_WriteReg(rddi::k_rddi_handle, rddi::k_rddi_if_index,
                                DAP_REG_AP_0x0 + (adr >> 2), val);
    status = SWD_CheckStatus(status);
    if (status)
        return (status);

    return (0);
}


#if DBGCM_V8M
// Update AP Security Access Parameters
//   attrib : Memory Access Attributes to evaluate
//   return value: error status
static int _UpdateAPSecAttr(DWORD attrib)
{
    DWORD       sectype;
    int         status;
    AP_CONTEXT *apCtx;

    // 27.06.2019: Updated AP handling
    status = AP_CurrentCtx(&apCtx);
    if (status)
        return (status);

    sectype = attrib & BLOCK_SECTYPE;

    switch (sectype) {
        case BLOCK_SECTYPE_ANY:
            if (apCtx->KeepSPROT) {
                break; // Skip SPROT update
            }
        case BLOCK_SECTYPE_CPU:
        case BLOCK_SECTYPE_SECURE:
            // Secure Access (BLOCK_SECTYPE_CPU for SBRSEL/SBRSELEN selection)
            // CSW_Val_Base &= ~CSW_SPROT;
            apCtx->CSW_Val_Base &= ~apCtx->SPROT;
            break;
        case BLOCK_SECTYPE_NSECURE:
            // Non-Secure Access
            // CSW_Val_Base |=  CSW_SPROT;
            apCtx->CSW_Val_Base |= apCtx->SPROT;
            break;
        default:
            return (EU12); // Unsupported Security Attribute
    }

    return (0);
}
#endif // DBGCM_V8M


// SWD Read 32-bit Data
//   adr    : Address
//   val    : Pointer to Value
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
int SWD_ReadD32(DWORD adr, DWORD *val, BYTE attrib)
{
#else  // DBGCM_V8M
int SWD_ReadD32(DWORD adr, DWORD *val)
{
#endif // DBGCM_V8M
    std::lock_guard<std::recursive_mutex> lk(kSWDOpMutex);

    int         status = 0;
    AP_CONTEXT *apCtx;

#if DBGCM_DBG_DESCRIPTION || DBGCM_DS_MONITOR
    int pstatus = 0;
#endif // DBGCM_DBG_DESCRIPTION || DBGCM_DS_MONITOR

#if DBGCM_DS_MONITOR
    BOOL dhcsr = FALSE;

    if (adr == DBG_HCSR) {
        dhcsr  = TRUE;
        status = DSM_SuspendMonitor();
        if (status)
            return (status);
    }
#endif // DBGCM_DS_MONITOR

    // 27.06.2019: Updated AP handling
    status = AP_Switch(&apCtx);
    if (status)
        goto end;

#if DBGCM_V8M
    status = SWD_UpdateDSCSR(adr, 4, attrib);
    if (status)
        goto end;

    status = _UpdateAPSecAttr(attrib);
    if (status)
        goto end;
#endif // DBGCM_V8M

    do {
        if (AP_Bank != 0) {
            status = SWD_WriteDP(DP_SELECT, AP_Sel | 0);
            if (status)
                break;
            AP_Bank = 0;
        }

        if ((apCtx->CSW_Val_Base & CSW_SIZE) != CSW_SIZE32) {
            apCtx->CSW_Val_Base &= ~CSW_SIZE;
            apCtx->CSW_Val_Base |= CSW_SIZE32;
            status = SWD_WriteAP(AP_CSW, apCtx->CSW_Val_Base);
            if (status)
                break;
        }

        status = SWD_ReadData(adr, val);
        if (status)
            break;
    } while (0);

    if (status == rddi::RDDI_DAP_ERROR_MEMORY) {
        status = EU14;
    }


    // See "Setting up target memory accesses based on AP_Context" above in this file for how
    // to construct the AP CSW value to write.

    // Extend error message with details if memory access failed
    if (status == EU14)
        SetStatusMem(EU14, adr, STATUS_MEMREAD, 4);
    if (status)
        goto end;

#if DBGCM_DBG_DESCRIPTION
    if (PDSCDebug_IsEnabled()) {
#if DBGCM_V8M
        pstatus = PDSCDebug_PatchData((U32)ACCESS_MEM, 4, adr, 4, (UC8 *)val, attrib);
        if (pstatus == 0) { // Error handling for data patches...
            status = 0;     // ... if data patched then clear previous access error.
        } else if (pstatus != EU38 && status == 0) {
            status = pstatus; // ... if error during patch and no previous error.
        }
        if (status)
            goto end;
#else  // DBGCM_V8M
        pstatus = PDSCDebug_PatchData((U32)ACCESS_MEM, 4, adr, 4, (UC8 *)val, 0 /*attrib*/);
        if (pstatus == 0) { // Error handling for data patches...
            status = 0;     // ... if data patched then clear previous access error.
        } else if (pstatus != EU38 && status == 0) {
            status = pstatus; // ... if error during patch and no previous error.
        }
        if (status)
            goto end;
#endif // DBGCM_V8M
    }
#endif // DBGCM_DBG_DESCRIPTION

#if DBGCM_DS_MONITOR
    if (adr == DBG_HCSR) {
        DSM_ExternalDHCSR(*val);
    }
#endif // DBGCM_DS_MONITOR

// #if DBGCM_DBG_DESCRIPTION || DBGCM_V8M
end:
    // #endif // DBGCM_DBG_DESCRIPTION || DBGCM_V8M

#if DBGCM_DS_MONITOR
    if (dhcsr) {
        if (status) {
            DSM_ResumeMonitor();
        } else {
            status = DSM_ResumeMonitor();
        }
    }
#endif // DBGCM_DS_MONITOR
    if (status)
        return (status);

    return (0);
}

// SWD Read 16-bit Data
//   adr    : Address
//   val    : Pointer to Value
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
int SWD_ReadD16(DWORD adr, WORD *val, BYTE attrib)
{
#else  // DBGCM_V8M
int SWD_ReadD16(DWORD adr, WORD *val)
{
#endif // DBGCM_V8M
    std::lock_guard<std::recursive_mutex> lk(kSWDOpMutex);

    // #if DBGCM_DBG_DESCRIPTION || DBGCM_V8M
    int   status = 0;
    DWORD v;
    // #endif // DBGCM_DBG_DESCRIPTION || DBGCM_V8M

#if DBGCM_DBG_DESCRIPTION
    int pstatus = 0;
#endif // DBGCM_DBG_DESCRIPTION

    AP_CONTEXT *apCtx;

    // 27.06.2019: Updated AP handling
    status = AP_Switch(&apCtx);
    if (status)
        return (status);

#if DBGCM_V8M
    status = SWD_UpdateDSCSR(adr, 2, attrib);
    if (status)
        return (status);

    status = _UpdateAPSecAttr(attrib);
    if (status)
        return (status);
#endif // DBGCM_V8M

    do {
        if (AP_Bank != 0) {
            status = SWD_WriteDP(DP_SELECT, AP_Sel | 0);
            if (status)
                break;
            AP_Bank = 0;
        }

        if ((apCtx->CSW_Val_Base & CSW_SIZE) != CSW_SIZE16) {
            apCtx->CSW_Val_Base &= ~CSW_SIZE;
            apCtx->CSW_Val_Base |= CSW_SIZE16;
            status = SWD_WriteAP(AP_CSW, apCtx->CSW_Val_Base);
            if (status)
                break;
        }

        status = SWD_ReadData(adr, &v);
        if (status)
            break;

        *val = (WORD)(v >> ((adr & 0x02) << 3));
    } while (0);

    if (status == rddi::RDDI_DAP_ERROR_MEMORY) {
        status = EU14;
    }

    // See "Setting up target memory accesses based on AP_Context" above in this file for how
    // to construct the AP CSW value to write.

    // Extend error message with details if memory access failed
    if (status == EU14)
        SetStatusMem(EU14, adr, STATUS_MEMREAD, 2);
    if (status)
        return (status);

#if DBGCM_DBG_DESCRIPTION
    if (PDSCDebug_IsEnabled()) {
        pstatus = PDSCDebug_PatchData((U32)ACCESS_MEM, 2, adr, 2, (UC8 *)val, 0 /*attrib*/);
        if (pstatus == 0) { // Error handling for data patches...
            status = 0;     // ... if data patched then clear previous access error.
        } else if (pstatus != EU38 && status == 0) {
            status = pstatus; // ... if error during patch and no previous error.
        }
        if (status)
            return (status);
    }
#endif // DBGCM_DBG_DESCRIPTION

    return (0);
}

// SWD Read 8-bit Data
//   adr    : Address
//   val    : Pointer to Value
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
int SWD_ReadD8(DWORD adr, BYTE *val, BYTE attrib)
{
#else  // DBGCM_V8M
int SWD_ReadD8(DWORD adr, BYTE *val)
{
#endif // DBGCM_V8M
    std::lock_guard<std::recursive_mutex> lk(kSWDOpMutex);

    // #if DBGCM_DBG_DESCRIPTION || DBGCM_V8M
    int   status = 0;
    DWORD v;
    // #endif // DBGCM_DBG_DESCRIPTION || DBGCM_V8M

#if DBGCM_DBG_DESCRIPTION
    int pstatus = 0;
#endif // DBGCM_DBG_DESCRIPTION

    AP_CONTEXT *apCtx;

    // 27.06.2019: Updated AP handling
    status = AP_Switch(&apCtx);
    if (status)
        return (status);

#if DBGCM_V8M
    status = SWD_UpdateDSCSR(adr, 1, attrib);
    if (status)
        return (status);

    status = _UpdateAPSecAttr(attrib);
    if (status)
        return (status);
#endif // DBGCM_V8M

    do {
        if (AP_Bank != 0) {
            status = SWD_WriteDP(DP_SELECT, AP_Sel | 0);
            if (status)
                break;
            AP_Bank = 0;
        }

        if ((apCtx->CSW_Val_Base & CSW_SIZE) != CSW_SIZE8) {
            apCtx->CSW_Val_Base &= ~CSW_SIZE;
            apCtx->CSW_Val_Base |= CSW_SIZE8;
            status = SWD_WriteAP(AP_CSW, apCtx->CSW_Val_Base);
            if (status)
                break;
        }

        status = SWD_ReadData(adr, &v);
        if (status)
            break;

        *val = (BYTE)(v >> ((adr & 0x03) << 3));
    } while (0);

    if (status == rddi::RDDI_DAP_ERROR_MEMORY) {
        status = EU14;
    }


    // See "Setting up target memory accesses based on AP_Context" above in this file for how
    // to construct the AP CSW value to write.

    // Extend error message with details if memory access failed
    if (status == EU14)
        SetStatusMem(EU14, adr, STATUS_MEMREAD, 1);
    if (status)
        return (status);

#if DBGCM_DBG_DESCRIPTION
    if (PDSCDebug_IsEnabled()) {
        pstatus = PDSCDebug_PatchData((U32)ACCESS_MEM, 1, adr, 1, (UC8 *)val, 0 /*attrib*/);
        if (pstatus == 0) { // Error handling for data patches...
            status = 0;     // ... if data patched then clear previous access error.
        } else if (pstatus != EU38 && status == 0) {
            status = pstatus; // ... if error during patch and no previous error.
        }
        if (status)
            return (status);
    }
#endif // DBGCM_DBG_DESCRIPTION

    return (0);
}


// SWD Write 32-bit Data
//   adr    : Address
//   val    : Value
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
int SWD_WriteD32(DWORD adr, DWORD val, BYTE attrib)
{
    // int status = 0;
#else  // DBGCM_V8M
int SWD_WriteD32(DWORD adr, DWORD val)
{
#endif // DBGCM_V8M
    std::lock_guard<std::recursive_mutex> lk(kSWDOpMutex);

    int         status = 0;
    AP_CONTEXT *apCtx;

    // 27.06.2019: Updated AP handling
    status = AP_Switch(&apCtx);
    if (status)
        return (status);

#if DBGCM_V8M
    status = SWD_UpdateDSCSR(adr, 4, attrib);
    if (status)
        return (status);

    status = _UpdateAPSecAttr(attrib);
    if (status)
        return (status);
#endif // DBGCM_V8M

    do {
        if (AP_Bank != 0) {
            status = SWD_WriteDP(DP_SELECT, AP_Sel | 0);
            if (status)
                break;
            AP_Bank = 0;
        }

        if ((apCtx->CSW_Val_Base & CSW_SIZE) != CSW_SIZE32) {
            apCtx->CSW_Val_Base &= ~CSW_SIZE;
            apCtx->CSW_Val_Base |= CSW_SIZE32;
            status = SWD_WriteAP(AP_CSW, apCtx->CSW_Val_Base);
            if (status)
                break;
        }

        status = SWD_WriteData(adr, val);
    } while (0);

    if (status == rddi::RDDI_DAP_ERROR_MEMORY) {
        status = EU14;
    }


    // See "Setting up target memory accesses based on AP_Context" above in this file for how
    // to construct the AP CSW value to write.

    // Extend error message with details if memory access failed
    if (status == EU14)
        SetStatusMem(EU14, adr, STATUS_MEMWRITE, 4);
    if (status)
        return (status);

    return (0);
}

// SWD Write 16-bit Data
//   adr    : Address
//   val    : Value
//   return value: error status
#if DBGCM_V8M
int SWD_WriteD16(DWORD adr, WORD val, BYTE attrib)
{
    // int status = 0;
#else  // DBGCM_V8M
int SWD_WriteD16(DWORD adr, WORD val)
{
#endif // DBGCM_V8M
    std::lock_guard<std::recursive_mutex> lk(kSWDOpMutex);

    int         status = 0;
    AP_CONTEXT *apCtx;

    // 27.06.2019: Updated AP handling
    status = AP_Switch(&apCtx);
    if (status)
        return (status);

#if DBGCM_V8M
    status = SWD_UpdateDSCSR(adr, 2, attrib);
    if (status)
        return (status);

    status = _UpdateAPSecAttr(attrib);
    if (status)
        return (status);
#endif // DBGCM_V8M

    do {
        if (AP_Bank != 0) {
            status = SWD_WriteDP(DP_SELECT, AP_Sel | 0);
            if (status)
                break;
            AP_Bank = 0;
        }

        if ((apCtx->CSW_Val_Base & CSW_SIZE) != CSW_SIZE16) {
            apCtx->CSW_Val_Base &= ~CSW_SIZE;
            apCtx->CSW_Val_Base |= CSW_SIZE16;
            status = SWD_WriteAP(AP_CSW, apCtx->CSW_Val_Base);
            if (status)
                break;
        }

        status = SWD_WriteData(adr, (DWORD)val << ((adr & 0x02) << 3));
    } while (0);

    if (status == rddi::RDDI_DAP_ERROR_MEMORY) {
        status = EU14;
    }


    // See "Setting up target memory accesses based on AP_Context" above in this file for how
    // to construct the AP CSW value to write.

    // Extend error message with details if memory access failed
    if (status == EU14)
        SetStatusMem(EU14, adr, STATUS_MEMWRITE, 2);
    if (status)
        return (status);

    return (0);
}

// SWD Write 8-bit Data
//   adr    : Address
//   val    : Value
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
int SWD_WriteD8(DWORD adr, BYTE val, BYTE attrib)
{
    // int status = 0;
#else  // DBGCM_V8M
int SWD_WriteD8(DWORD adr, BYTE val)
{
#endif // DBGCM_V8M
    std::lock_guard<std::recursive_mutex> lk(kSWDOpMutex);

    int         status = 0;
    AP_CONTEXT *apCtx;

    // 27.06.2019: Updated AP handling
    status = AP_Switch(&apCtx);
    if (status)
        return (status);

#if DBGCM_V8M
    status = SWD_UpdateDSCSR(adr, 1, attrib);
    if (status)
        return (status);

    status = _UpdateAPSecAttr(attrib);
    if (status)
        return (status);
#endif // DBGCM_V8M

    do {
        if (AP_Bank != 0) {
            status = SWD_WriteDP(DP_SELECT, AP_Sel | 0);
            if (status)
                break;
            AP_Bank = 0;
        }

        if ((apCtx->CSW_Val_Base & CSW_SIZE) != CSW_SIZE8) {
            apCtx->CSW_Val_Base &= ~CSW_SIZE;
            apCtx->CSW_Val_Base |= CSW_SIZE8;
            status = SWD_WriteAP(AP_CSW, apCtx->CSW_Val_Base);
            if (status)
                break;
        }

        status = SWD_WriteData(adr, (DWORD)val << ((adr & 0x03) << 3));
    } while (0);


    if (status == rddi::RDDI_DAP_ERROR_MEMORY) {
        status = EU14;
    }


    // See "Setting up target memory accesses based on AP_Context" above in this file for how
    // to construct the AP CSW value to write.

    // Extend error message with details if memory access failed
    if (status == EU14)
        SetStatusMem(EU14, adr, STATUS_MEMWRITE, 1);
    if (status)
        return (status);

    return (0);
}


// SWD Read Data Block (32-bit Elements inside R/W Page Block)
// Block parameters 'adr' and 'nMany' must be 4-Byte aligned to allow an efficient implementation.
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
int SWD_ReadBlock(DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib)
{
    std::lock_guard<std::recursive_mutex> lk(kSWDOpMutex);

    int         status = 0;
    AP_CONTEXT *apCtx;
    DWORD       rwpage;

#if DBGCM_DBG_DESCRIPTION || DBGCM_DS_MONITOR
    int pstatus = 0;
#endif // DBGCM_DBG_DESCRIPTION || DBGCM_DS_MONITOR

#if DBGCM_DS_MONITOR
    BOOL dhcsr = FALSE;
#endif // DBGCM_DS_MONITOR

    if (nMany == 0)
        return (EU01);
    if (nMany & 0x03)
        return (EU01);
        // if (nMany > RWPage) return (EU01);

#if DBGCM_DS_MONITOR
    if (adr <= DBG_HCSR && (adr + nMany) > DBG_HCSR) {
        dhcsr  = TRUE;
        status = DSM_SuspendMonitor();
        if (status)
            return (status);
    }
#endif // DBGCM_DS_MONITOR

    if (!(attrib & BLOCK_NADDRINC)) {
        rwpage = AP_CurrentRWPage(); // Get effective RWPage based on DP/AP selection
        if (nMany > rwpage) {
            status = (EU01);
            goto end;
        }
    }

    // 27.06.2019: Updated AP handling
    status = AP_Switch(&apCtx);
    if (status)
        goto end;

#if DBGCM_V8M
    status = SWD_UpdateDSCSR(adr, nMany, attrib);
    if (status)
        goto end;

    status = _UpdateAPSecAttr(attrib);
    if (status)
        goto end;
#endif // DBGCM_V8M

    assert(attrib == 0);

    do {
        if (AP_Bank != 0) {
            status = SWD_WriteDP(DP_SELECT, AP_Sel | 0);
            if (status)
                break;
            AP_Bank = 0;
        }

        if ((apCtx->CSW_Val_Base & (CSW_SIZE | CSW_ADDRINC)) != (CSW_SIZE32 | CSW_SADDRINC)) {
            apCtx->CSW_Val_Base &= ~(CSW_SIZE | CSW_ADDRINC);
            apCtx->CSW_Val_Base |= (CSW_SIZE32 | CSW_SADDRINC);
            status = SWD_WriteAP(AP_CSW, apCtx->CSW_Val_Base);
            if (status)
                break;
        }

        status = SWD_WriteAP(AP_TAR, adr);
        if (status)
            break;

        // Multiple Read AP DRW
        status = rddi::DAP_RegReadRepeat(rddi::k_rddi_handle, 0,
                                         nMany >> 2, DAP_AP_REG_DRW, (int *)pB);
        status = SWD_CheckStatus(status);
        if (status)
            break;

        status = SWD_StickyError();
        if (status)
            break;
    } while (0);


    if (status == rddi::RDDI_DAP_ERROR_MEMORY) {
        status = EU14;
    }

    // See "Setting up target memory accesses based on AP_Context" above in this file for how
    // to construct the AP CSW value to write.

    // Extend error message with details if memory access failed
    // Ideally use the actual address of the failing access
    if (status == EU14)
        SetStatusMem(EU14, adr, STATUS_MEMREAD, 4);
    if (status)
        goto end;

#if DBGCM_DBG_DESCRIPTION
    if (PDSCDebug_IsEnabled()) {
        pstatus = PDSCDebug_PatchData((U32)ACCESS_MEM, 4, adr, nMany, (UC8 *)pB, 0 /*attrib*/);
        if (pstatus == 0) { // Error handling for data patches...
            status = 0;     // ... if data patched then clear previous access error.
        } else if (pstatus != EU38 && status == 0) {
            status = pstatus; // ... if error during patch and no previous error.
        }
        if (status)
            goto end;
    }
#endif // DBGCM_DBG_DESCRIPTION

#if DBGCM_DS_MONITOR
    if (dhcsr) {
        DSM_ExternalDHCSR(*((DWORD *)(pB + (DBG_HCSR - adr))));
    }
#endif // DBGCM_DS_MONITOR

// #if DBGCM_DBG_DESCRIPTION || DBGCM_V8M
end:
    // #endif // DBGCM_DBG_DESCRIPTION || DBGCM_V8M

#if DBGCM_DS_MONITOR
    if (dhcsr) {
        if (status) {
            DSM_ResumeMonitor();
        } else {
            status = DSM_ResumeMonitor();
        }
    }
#endif // DBGCM_DS_MONITOR

    return (status);
}


// SWD Write Data Block (32-bit Elements inside R/W Page Block)
// Block parameters 'adr' and 'nMany' must be 4-Byte aligned to allow an efficient implementation.
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
int SWD_WriteBlock(DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib)
{
    std::lock_guard<std::recursive_mutex> lk(kSWDOpMutex);

    // #if DBGCM_V8M
    int status = 0;
    // #endif // DBGCM_V8M

    AP_CONTEXT *apCtx;
    DWORD       rwpage;

    if (nMany == 0)
        return (EU01);
    if (nMany & 0x03)
        return (EU01);
    // if (nMany > RWPage) return (EU01);

    if (!(attrib & BLOCK_NADDRINC)) {
        rwpage = AP_CurrentRWPage(); // Get effective RWPage based on DP/AP selection
        if (nMany > rwpage)
            return (EU01);
    }

    // 27.06.2019: Updated AP handling
    status = AP_Switch(&apCtx);
    if (status)
        return (status);

#if DBGCM_V8M
    status = SWD_UpdateDSCSR(adr, nMany, attrib);
    if (status)
        return (status);

    status = _UpdateAPSecAttr(attrib);
    if (status)
        return (status);
#endif // DBGCM_V8M

    do {
        if (AP_Bank != 0) {
            status = SWD_WriteDP(DP_SELECT, AP_Sel | 0);
            if (status)
                break;
            AP_Bank = 0;
        }

        if ((apCtx->CSW_Val_Base & (CSW_SIZE | CSW_ADDRINC)) != (CSW_SIZE32 | CSW_SADDRINC)) {
            apCtx->CSW_Val_Base &= ~(CSW_SIZE | CSW_ADDRINC);
            apCtx->CSW_Val_Base |= (CSW_SIZE32 | CSW_SADDRINC);
            status = SWD_WriteAP(AP_CSW, apCtx->CSW_Val_Base);
            if (status)
                break;
        }

        status = SWD_WriteAP(AP_TAR, adr);
        if (status)
            break;

        // Multiple Write AP DRW
        status = rddi::DAP_RegWriteRepeat(rddi::k_rddi_handle, 0,
                                          nMany >> 2, DAP_AP_REG_DRW, (int *)pB);
        status = SWD_CheckStatus(status);
        if (status)
            break;

        status = SWD_StickyError();
        if (status)
            break;
    } while (0);

    if (status == rddi::RDDI_DAP_ERROR_MEMORY) {
        status = EU14;
    }


    // See "Setting up target memory accesses based on AP_Context" above in this file for how
    // to construct the AP CSW value to write.

    // Extend error message with details if memory access failed
    // Ideally use the actual address of the failing access
    if (status == EU14)
        SetStatusMem(EU14, adr, STATUS_MEMWRITE, 4);
    if (status)
        return (status);

    return (0);
}


// SWD Verify Data Block (32-bit Elements inside R/W Page Block)
// Block parameters 'adr' and 'nMany' must be 4-Byte aligned to allow an efficient implementation.
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status or -1 on Verify Missmatch
#if DBGCM_V8M
int SWD_VerifyBlock(DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib)
{
    // int status = 0;
#else  // DBGCM_V8M
int SWD_VerifyBlock(DWORD adr, BYTE *pB, DWORD nMany)
{
#endif // DBGCM_V8M
    std::lock_guard<std::recursive_mutex> lk(kSWDOpMutex);

    int   status = 0;
    int   flag   = 0;
    DWORD rwpage, val;

    AP_CONTEXT *apCtx;

    if (nMany == 0)
        return (EU01);
    if (nMany & 0x03)
        return (EU01);
    // if (nMany > RWPage) return (EU01);

    rwpage = AP_CurrentRWPage();
    if (nMany > rwpage)
        return (EU01);

    status = AP_Switch(&apCtx);
    if (status)
        return (status);

#if DBGCM_V8M
    status = SWD_UpdateDSCSR(adr, nMany, attrib);
    if (status)
        return (status);

    status = _UpdateAPSecAttr(attrib);
    if (status)
        return (status);
#endif // DBGCM_V8M

    // See "Setting up target memory accesses based on AP_Context" above in this file for how
    // to construct the AP CSW value to write.

    if (AP_Bank != 0) {
        status = SWD_WriteDP(DP_SELECT, AP_Sel | 0);
        if (status)
            goto fail;
        AP_Bank = 0;
    }

    if ((apCtx->CSW_Val_Base & (CSW_SIZE | CSW_ADDRINC)) != (CSW_SIZE32 | CSW_SADDRINC)) {
        apCtx->CSW_Val_Base &= ~(CSW_SIZE | CSW_ADDRINC);
        apCtx->CSW_Val_Base |= (CSW_SIZE32 | CSW_SADDRINC);
        status = SWD_WriteAP(AP_CSW, apCtx->CSW_Val_Base);
        if (status)
            goto fail;
    }

    status = SWD_WriteAP(AP_TAR, adr);
    if (status)
        goto fail;

    // Configure pushed compare
    status = SWD_ReadDP(DP_CTRL_STAT, &val);
    if (status)
        goto fail;
    val &= ~TRNMODE;
    val |= TRNVERIFY | STICKYCMP;
    status = SWD_WriteDP(DP_CTRL_STAT, val);
    if (status)
        goto fail;

    flag   = 0;
    status = rddi::DAP_RegWriteRepeat(rddi::k_rddi_handle, 0,
                                      nMany >> 2, DAP_AP_REG_DRW, (int *)pB);

    if (status == RDDI_DAP_OPERATION_TIMEOUT) {
        status = SWD_DAPAbortVal(DAPABORT);
        if (status == 0) {
            status = rddi::RDDI_DAP_ERROR_MEMORY;
        }
    } else if (status == RDDI_DAP_DP_STICKY_ERR) {
        // Only for SW (not availalbe for JTAG)
        status = SWD_ReadDP(DP_CTRL_STAT, &val);
        if (status == 0) { // ReadDP OK
            status = SWD_DAPAbortVal(STKERRCLR | STICKYCMP | WDERRCLR);
            if (status == 0) { // Abort OK
                if (val & (STICKYERR | WDATAERR)) {
                    status = rddi::RDDI_DAP_ERROR_MEMORY;
                }
                if (val & STICKYCMP) {
                    flag = -1; // Verify Mismatch (JTAG)
                }
            }
        }
    } else if (status == RDDI_SUCCESS) {
        status = SWD_ReadDP(DP_CTRL_STAT, &val);
        if (status == 0) { // ReadDP OK
            if (val & STICKYERR) {
                status = rddi::RDDI_DAP_ERROR_MEMORY;
            }
            if (val & STICKYCMP) {
                flag = -1; // Verify Mismatch (JTAG)
            }
        }
    } else {
        status = rddi::RDDI_DAP_ERROR;
    }


    val &= ~TRNMODE;
    if (status) {
        SWD_WriteDP(DP_CTRL_STAT, val);
    } else {
        status = SWD_WriteDP(DP_CTRL_STAT, val);
    }


fail:
    if (status == rddi::RDDI_DAP_ERROR_MEMORY) {
        return EU14;
    } else if (status != 0) {
        return EU01;
    }

    return (flag);
}


// SWD Read ARM Memory
// No aligment required for access parameters 'nAdr' and 'nMany'
//   nAdr   : Start Address (used also to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes to Read
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
int SWD_ReadARMMem(DWORD *nAdr, BYTE *pB, DWORD nMany, BYTE attrib)
{
#else  // DBGCM_V8M
int SWD_ReadARMMem(DWORD *nAdr, BYTE *pB, DWORD nMany)
{
#endif // DBGCM_V8M
    std::lock_guard<std::recursive_mutex> lk(kSWDOpMutex);

    int   status   = 0;
    int   acc_size = 0;
    DWORD rwpage;
    DWORD n;

    rwpage = AP_CurrentRWPage(); // Get effective RWPage based on DP/AP selection
    assert(attrib == 0);

    // Read 8-bit Data (8-bit Aligned)
    if ((*nAdr & 0x01) && nMany) {
        acc_size = 1;
        status   = SWD_ReadD8(*nAdr, pB, attrib);
        if (status)
            goto out;
        status = SWD_StickyError();
        if (status)
            goto out;
        pB += 1;
        *nAdr += 1;
        nMany -= 1;
    }

    // Read 16-bit Data (16-bit Aligned)
    if ((*nAdr & 0x02) && (nMany >= 2)) {
        acc_size = 2;
        status   = SWD_ReadD16(*nAdr, (WORD *)pB, attrib);
        if (status)
            goto out;
        status = SWD_StickyError();
        if (status)
            goto out;
        pB += 2;
        *nAdr += 2;
        nMany -= 2;
    }

    // Read Data Block (32-bit Aligned)
    while (nMany >= 4) {
        acc_size = 4;
        n        = rwpage - (*nAdr & (rwpage - 1));
        if (nMany < n)
            n = nMany & 0xFFFFFFFC;
        status = SWD_ReadBlock(*nAdr, pB, n, attrib);
        if (status == rddi::RDDI_DAP_ERROR_MEMORY || status == EU14) {
            // Slow Access
            while (n) {
                status = SWD_ReadD32(*nAdr, (DWORD *)pB, attrib);
                if (status)
                    goto out;
                pB += 4;
                *nAdr += 4;
                nMany -= 4;
                n -= 4;
            }
            status = SWD_StickyError();
            if (status)
                goto out;
            continue;
        }
        if (status)
            goto out;
        pB += n;
        *nAdr += n;
        nMany -= n;
    }

    // Read 16-bit Data (16-bit Aligned)
    if (nMany >= 2) {
        acc_size = 2;
        status   = SWD_ReadD16(*nAdr, (WORD *)pB, attrib);
        if (status)
            goto out;
        status = SWD_StickyError();
        if (status)
            goto out;
        pB += 2;
        *nAdr += 2;
        nMany -= 2;
    }

    // Read 8-bit Data (8-bit Aligned)
    if (nMany) {
        acc_size = 1;
        status   = SWD_ReadD8(*nAdr, pB, attrib);
        if (status)
            goto out;
        status = SWD_StickyError();
        if (status)
            goto out;
        pB += 1;
        *nAdr += 1;
        nMany -= 1;
    }


out:
    if (rddi::RDDI_DAP_ERROR_MEMORY == status) {
        status = EU14;
    }

    // No requirement to how the target memory is read. Can be for example a combination of 8, 16, and
    // 32 Bit accesses. It is valid to call other access functions implemented in this source file.

    // Extend error message with details if memory access failed
    // Ideally use the actual address of the failing access and adjust the size parameter according to
    // the executed access
    if (status == EU14)
        SetStatusMem(EU14, *nAdr, STATUS_MEMREAD, acc_size);
    if (status)
        return (status);

    return (0);
}


// SWD Write ARM Memory
// No aligment required for access parameters 'nAdr' and 'nMany'
//   nAdr   : Start Address (used also to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes to Write
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
int SWD_WriteARMMem(DWORD *nAdr, BYTE *pB, DWORD nMany, BYTE attrib)
{
#else  // DBGCM_V8M
int SWD_WriteARMMem(DWORD *nAdr, BYTE *pB, DWORD nMany)
{
#endif // DBGCM_V8M
    std::lock_guard<std::recursive_mutex> lk(kSWDOpMutex);

    int   status   = 0;
    int   acc_size = 0;
    DWORD rwpage;
    DWORD n;

    rwpage = AP_CurrentRWPage(); // Get effective RWPage based on DP/AP selection
    assert(attrib == 0);

    // Write 8-bit Data (8-bit Aligned)
    if ((*nAdr & 0x01) && nMany) {
        acc_size = 1;
        status   = SWD_WriteD8(*nAdr, *pB, attrib);
        if (status)
            goto out;
        status = SWD_StickyError();
        if (status)
            goto out;
        pB += 1;
        *nAdr += 1;
        nMany -= 1;
    }

    // Write 16-bit Data (16-bit Aligned)
    if ((*nAdr & 0x02) && (nMany >= 2)) {
        acc_size = 2;
        status   = SWD_WriteD16(*nAdr, *((WORD *)pB), attrib);
        if (status)
            goto out;
        status = SWD_StickyError();
        if (status)
            goto out;
        pB += 2;
        *nAdr += 2;
        nMany -= 2;
    }

    // Write Data Block (32-bit Aligned)
    while (nMany >= 4) {
        acc_size = 4;
        n        = rwpage - (*nAdr & (rwpage - 1));
        if (nMany < n)
            n = nMany & 0xFFFFFFFC;
        status = SWD_WriteBlock(*nAdr, pB, n, attrib);
        if (status == rddi::RDDI_DAP_ERROR_MEMORY || status == EU14) {
            // Slow Access
            while (n) {
                status = SWD_WriteD32(*nAdr, *((DWORD *)pB), attrib);
                if (status)
                    goto out;
                pB += 4;
                *nAdr += 4;
                nMany -= 4;
                n -= 4;
            }
            status = SWD_StickyError();
            if (status)
                goto out;
            continue;
        }
        if (status)
            goto out;
        pB += n;
        *nAdr += n;
        nMany -= n;
    }

    // Write 16-bit Data (16-bit Aligned)
    if (nMany >= 2) {
        acc_size = 2;
        status   = SWD_WriteD16(*nAdr, *((WORD *)pB), attrib);
        if (status)
            goto out;
        status = SWD_StickyError();
        if (status)
            goto out;
        pB += 2;
        *nAdr += 2;
        nMany -= 2;
    }

    // Write 8-bit Data (8-bit Aligned)
    if (nMany) {
        acc_size = 1;
        status   = SWD_WriteD8(*nAdr, *pB, attrib);
        if (status)
            goto out;
        status = SWD_StickyError();
        if (status)
            goto out;
        pB += 1;
        *nAdr += 1;
        nMany -= 1;
    }

out:
    if (rddi::RDDI_DAP_ERROR_MEMORY == status) {
        status = EU14;
    }


    // No requirement to how the target memory is written. Can be for example a combination of 8, 16, and
    // 32 Bit accesses. It is valid to call other access functions implemented in this source file.

    // Extend error message with details if memory access failed
    // Ideally use the actual address of the failing access and adjust the size parameter according to
    // the executed access
    if (status == EU14)
        SetStatusMem(EU14, *nAdr, STATUS_MEMWRITE, acc_size);
    if (status)
        return (status);

    return (0);
}


// SWD Verify ARM Memory
// No aligment required for access parameters 'nAdr' and 'nMany'
//   nAdr   : Start Address (used also to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of bytes to Verify
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
#if DBGCM_V8M
int SWD_VerifyARMMem(DWORD *nAdr, BYTE *pB, DWORD nMany, BYTE attrib)
{
#else  // DBGCM_V8M
int SWD_VerifyARMMem(DWORD *nAdr, BYTE *pB, DWORD nMany)
{
#endif // DBGCM_V8M
    std::lock_guard<std::recursive_mutex> lk(kSWDOpMutex);

    // No requirement to how the target memory is verified. Can be for example a combination of 8, 16, and
    // 32 Bit accesses. It is valid to call other access functions implemented in this source file.
    int   status;
    DWORD n;
    DWORD rwpage;

    rwpage = AP_CurrentRWPage(); // Get effective RWPage based on DP/AP selection

    assert(attrib == 0);

    // Read 8-bit Data (8-bit Aligned)
    if ((*nAdr & 0x01) && nMany) {
        status = SWD_ReadD8(*nAdr, pB, attrib);
        if (status)
            goto out;
        status = SWD_StickyError();
        if (status)
            goto out;
        pB += 1;
        *nAdr += 1;
        nMany -= 1;
    }

    // Read 16-bit Data (16-bit Aligned)
    if ((*nAdr & 0x02) && (nMany >= 2)) {
        status = SWD_ReadD16(*nAdr, (WORD *)pB, attrib);
        if (status)
            goto out;
        status = SWD_StickyError();
        if (status)
            goto out;
        pB += 2;
        *nAdr += 2;
        nMany -= 2;
    }

    // Pushed Verify Data Block (32-bit Aligned)
    while (nMany >= 4) {
        n = rwpage - (*nAdr & (rwpage - 1));
        if (nMany < n)
            n = nMany & 0xFFFFFFFC;
        status = SWD_VerifyBlock(*nAdr, pB, n, attrib);
        if (status == -1) {
            // Verify failed -> Detect 1st missmatch
            status = SWD_ReadBlock(*nAdr, pB, n, attrib);
            if (status)
                goto out;
            return (0);
        }
        if (status == rddi::RDDI_DAP_ERROR_MEMORY || status == EU14) {
            // Slow Access
            while (n) {
                status = SWD_ReadD32(*nAdr, (DWORD *)pB, attrib);
                if (status)
                    goto out;
                pB += 4;
                *nAdr += 4;
                nMany -= 4;
                n -= 4;
            }
            status = SWD_StickyError();
            if (status)
                goto out;
            continue;
        }
        if (status)
            goto out;
        pB += n;
        *nAdr += n;
        nMany -= n;
    }

    // Read 16-bit Data (16-bit Aligned)
    if (nMany >= 2) {
        status = SWD_ReadD16(*nAdr, (WORD *)pB, attrib);
        if (status)
            goto out;
        status = SWD_StickyError();
        if (status)
            goto out;
        pB += 2;
        *nAdr += 2;
        nMany -= 2;
    }

    // Read 8-bit Data (8-bit Aligned)
    if (nMany) {
        status = SWD_ReadD8(*nAdr, pB, attrib);
        if (status)
            goto out;
        status = SWD_StickyError();
        if (status)
            goto out;
        pB += 1;
        *nAdr += 1;
        nMany -= 1;
    }

out:
    if (rddi::RDDI_DAP_ERROR_MEMORY == status) {
        status = EU14;
    }

    return status;
}


// SWD Get ARM Registers
//   regs   : Pointer to ARM Registers
//   rfpu   : Pointer to FPU Registers
//   rsec   : Pointer to v8-M Security Extension Registers
//   mask   : Register Mask
//            Bit  0..15 : R0..R15
//            Bit     16 : xPSR
//            Bit     17 : MSP
//            Bit     18 : PSP
//            Bit     19 : Reserved
//            Bit     20 : SYS (i.e. CONTROL + FAULTMASK + BASEPRI + PRIMASK)
//            Bit     21 : MSP_NS
//            Bit     22 : PSP_NS
//            Bit     23 : MSP_S
//            Bit     24 : PSP_S
//            Bit     25 : MSPLIM_S
//            Bit     26 : PSPLIM_S
//            Bit     27 : MSPLIM_NS
//            Bit     28 : PSPLIM_NS
//            Bit     29 : SYS_S (siehe Bit 20)
//            Bit     30 : SYS_NS (siehe Bit 20)
//            Bit     31 : FPSCR
//            Bit 32..63 : S0..S31
//   return value: error status
#if DBGCM_V8M
int SWD_GetARMRegs(RgARMCM *regs, RgARMFPU *rfpu, RgARMV8MSE *rsec, U64 mask)
{
#else  // DBGCM_V8M
int SWD_GetARMRegs(RgARMCM *regs, RgARMFPU *rfpu, U64 mask)
{
#endif // DBGCM_V8M
    std::lock_guard<std::recursive_mutex> lk(kSWDOpMutex);

    if (mask == 0)
        return (EU01);

    int   status;
    int   regID[3 * 64];
    int   regData[3 * 64];
    int   i, n, m;
    DWORD val;

    // Match Retry = 100
    regID[0]   = DAP_REG_MATCH_RETRY;
    regData[0] = 100;

    // Match Mask = 0x00010000
    regID[1]   = DAP_REG_MATCH_MASK;
    regData[1] = 0x00010000;

    // SELECT = AP_Sel
    regID[2]   = DAP_DP_REG_APSEL;
    regData[2] = AP_Sel;

    // TAR = DBG_Addr
    regID[3]   = DAP_AP_REG_TAR;
    regData[3] = DBG_Addr;

    // SELECT = AP_Sel | 0x10
    regID[4]   = DAP_DP_REG_APSEL;
    regData[4] = AP_Sel | 0x10;

    // R/W DAP Registers
    status = rddi::DAP_RegAccessBlock(rddi::k_rddi_handle, 0, 5, regID, regData);
    status = SWD_CheckStatus(status);
    if (status) {
        goto fail;
    }


    AP_Bank = 0x10;

    // Prepare Register Access
    for (i = 0, n = 0; n < 64; n++) {
        if (mask & (1ULL << n)) {
            // Get register selector
            if (n < 21) {
                m = n; // Core Registers
            } else if (n >= 32) {
                m = 64 + (n - 32); // FPU Sn
            } else if (n == 31) {
                m = 33; // FPU FPCSR
            } else {
                continue;
            }

            // Select register to read (write to DCRSR)
            regID[i + 0]   = DAP_REG_AP_0x4;
            regData[i + 0] = m;
            // Read and wait for register ready flag (read DHCSR.16)
            regID[i + 1]   = DAP_REG_AP_0x0 | DAP_REG_RnW | DAP_REG_WaitForValue;
            regData[i + 1] = 0x00010000; // Value to Match
            // Read register value (read DCRDR)
            regID[i + 2] = DAP_REG_AP_0x8 | DAP_REG_RnW;

            i += 3;
        }
    }

    // R/W DAP Registers
    status = rddi::DAP_RegAccessBlock(rddi::k_rddi_handle, 0, i, regID, regData);
    status = SWD_CheckStatus(status);
    if (status)
        goto fail;

    status = SWD_StickyError();
    if (status)
        goto fail;

    // Store register values
    for (i = 0, n = 0; n < 64; n++) {
        if (mask & (1ULL << n)) {
            val = regData[i + 2];
            i += 3;
            if ((n < 21) && regs) {
                *((DWORD *)regs + n) = val;
            } else if ((n >= 32) && rfpu) {
                *((DWORD *)rfpu + (n - 32)) = val;
            } else if ((n == 31) && rfpu) {
                rfpu->FPSCR = val;
            }
        }
    }

    return (0);

fail:
    if (status == rddi::RDDI_DAP_ERROR_MEMORY) {
        return EU14;
    } else {
        return EU01;
    }
}


// SWD Set ARM Registers
//   regs   : Pointer to ARM Registers
//   rfpu   : Pointer to FPU Registers
//   rsec   : Pointer to v8-M Security Extension Registers
//   mask   : Register Mask
//            Bit  0..15 : R0..R15
//            Bit     16 : xPSR
//            Bit     17 : MSP
//            Bit     18 : PSP
//            Bit     19 : Reserved
//            Bit     20 : SYS (i.e. CONTROL + FAULTMASK + BASEPRI + PRIMASK)
//            Bit     21 : MSP_NS
//            Bit     22 : PSP_NS
//            Bit     23 : MSP_S
//            Bit     24 : PSP_S
//            Bit     25 : MSPLIM_S
//            Bit     26 : PSPLIM_S
//            Bit     27 : MSPLIM_NS
//            Bit     28 : PSPLIM_NS
//            Bit     29 : SYS_S (siehe Bit 20)
//            Bit     30 : SYS_NS (siehe Bit 20)
//            Bit     31 : FPSCR
//            Bit 32..63 : S0..S31
//   return value: error status
#if DBGCM_V8M
int SWD_SetARMRegs(RgARMCM *regs, RgARMFPU *rfpu, RgARMV8MSE *rsec, U64 mask)
{
#else  // DBGCM_V8M
int SWD_SetARMRegs(RgARMCM *regs, RgARMFPU *rfpu, U64 mask)
{
#endif // DBGCM_V8M
    std::lock_guard<std::recursive_mutex> lk(kSWDOpMutex);

    if (mask == 0)
        return (EU01);

    int   status;
    int   regID[3 * 64];
    int   regData[3 * 64];
    int   i, n, m;
    DWORD val;

    // Match Retry = 100
    regID[0]   = DAP_REG_MATCH_RETRY;
    regData[0] = 100;

    // Match Mask = 0x00010000
    regID[1]   = DAP_REG_MATCH_MASK;
    regData[1] = 0x00010000;

    // SELECT = AP_Sel
    regID[2]   = DAP_DP_REG_APSEL;
    regData[2] = AP_Sel;

    // TAR = DBG_Addr
    regID[3]   = DAP_AP_REG_TAR;
    regData[3] = DBG_Addr;

    // SELECT = AP_Sel | 0x10
    regID[4]   = DAP_DP_REG_APSEL;
    regData[4] = AP_Sel | 0x10;

    // R/W DAP Registers
    status = rddi::DAP_RegAccessBlock(rddi::k_rddi_handle, 0, 5, regID, regData);
    status = SWD_CheckStatus(status);
    if (status)
        goto fail;


    AP_Bank = 0x10;

    // Prepare Register Access
    for (i = 0, n = 0; n < 64; n++) {
        if (mask & (1ULL << n)) {
            // Get register selector and Load register value
            if ((n < 21) && regs) {
                m   = 0x00010000 | n; // Core Registers
                val = *((DWORD *)regs + n);
            } else if ((n >= 32) && rfpu) {
                m   = 0x00010000 | (64 + (n - 32)); // FPU Sn
                val = *((DWORD *)rfpu + (n - 32));
            } else if ((n == 31) && rfpu) {
                m   = 0x00010000 | 33; // FPU FPCSR
                val = rfpu->FPSCR;
            } else {
                continue;
            }

            // Write register value (write to DCRDR)
            regID[i + 0]   = DAP_REG_AP_0x8;
            regData[i + 0] = val;
            // Select register to write (write to DCRSR)
            regID[i + 1]   = DAP_REG_AP_0x4;
            regData[i + 1] = 0x00010000 | m;
            // Read and wait for register ready flag (read DHCSR.16)
            regID[i + 2]   = DAP_REG_AP_0x0 | DAP_REG_RnW | DAP_REG_WaitForValue;
            regData[i + 2] = 0x00010000; // Value to Match

            i += 3;
        }
    }

    // R/W DAP Registers
    status = rddi::DAP_RegAccessBlock(rddi::k_rddi_handle, 0, i, regID, regData);
    status = SWD_CheckStatus(status);
    if (status)
        goto fail;

    status = SWD_StickyError();
    if (status)
        goto fail;

    return (0);

fail:
    if (status == rddi::RDDI_DAP_ERROR_MEMORY) {
        return EU14;
    } else {
        return EU01;
    }
}


// SWD Execute System Call
//   regs   : Pointer to ARM Registers
//   return value: error status
int SWD_SysCallExec(RgARMCM *regs)
{
    std::lock_guard<std::recursive_mutex> lk(kSWDOpMutex);

    int   status;
    int   regID[3 * 16];
    int   regData[3 * 16];
    int   i, n;
    DWORD mask;


    // Match Retry = 100
    regID[0]   = DAP_REG_MATCH_RETRY;
    regData[0] = 100;

    // Match Mask = 0x00010000
    regID[1]   = DAP_REG_MATCH_MASK;
    regData[1] = 0x00010000;

    // TAR = DBG_Addr
    regID[2]   = DAP_AP_REG_TAR;
    regData[2] = DBG_Addr;

    // SELECT = AP_Sel | 0x10
    regID[3]   = DAP_DP_REG_APSEL;
    regData[3] = AP_Sel | 0x10;

    // R/W DAP Registers
    status = rddi::DAP_RegAccessBlock(rddi::k_rddi_handle, 0, 4, regID, regData);
    status = SWD_CheckStatus(status);
    if (status)
        goto fail;

    AP_Bank = 0x10;

    // Register mask
    mask = (1UL << 0) |  // R0 (A1)
           (1UL << 1) |  // R1 (A2)
           (1UL << 2) |  // R2 (A3)
           (1UL << 3) |  // R3 (A4)
           (1UL << 9) |  // R9 (SB)
           (1UL << 13) | // R13(SP)
           (1UL << 14) | // R14(LR)
           (1UL << 15) | // R15(PC)
           (1UL << 16);  // xPSR

    // Prepare Register Access
    for (i = 0, n = 0; n <= 16; n++) {
        if (mask & (1UL << n)) {
            // Write register value (write to DCRDR)
            regID[i + 0]   = DAP_REG_AP_0x8;
            regData[i + 0] = *((DWORD *)regs + n);
            // Select register to write (write to DCRSR)
            regID[i + 1]   = DAP_REG_AP_0x4;
            regData[i + 1] = 0x00010000 | n;
            // Read and wait for register ready flag (read DHCSR.16)
            regID[i + 2]   = DAP_REG_AP_0x0 | DAP_REG_RnW | DAP_REG_WaitForValue;
            regData[i + 2] = 0x00010000; // Value to Match
            i += 3;
        }
    }

    // DHCSR = DBGKEY | C_DEBUGEN
    regID[i]   = DAP_REG_AP_0x0;
    regData[i] = DBGKEY | C_DEBUGEN;
    i++;

    // DP_CTRL_STAT read
    regID[i] = DAP_REG_DP_0x4 | DAP_REG_RnW;

    // R/W DAP Registers
    status = rddi::DAP_RegAccessBlock(rddi::k_rddi_handle, 0, i + 1, regID, regData);
    status = SWD_CheckStatus(status);
    if (status)
        goto fail;

    status = SWD_CheckStickyError(regData[i]);

fail:
    if (status == rddi::RDDI_DAP_ERROR_MEMORY) {
        return EU14;
    } else if (status != 0) {
        return EU01;
    }

    return (0);
}


// SWD Read System Call Result
//   rval   : Pointer to Result Value
//   return value: error status
int SWD_SysCallRes(DWORD *rval)
{
    std::lock_guard<std::recursive_mutex> lk(kSWDOpMutex);

    int status;
    int regID[4];
    int regData[4];


    // Match Retry = 100
    regID[0]   = DAP_REG_MATCH_RETRY;
    regData[0] = 100;

    // Match Mask = 0x00010000
    regID[1]   = DAP_REG_MATCH_MASK;
    regData[1] = 0x00010000;

    // TAR = DBG_Addr
    regID[2]   = DAP_AP_REG_TAR;
    regData[2] = DBG_Addr;

    // SELECT = AP_Sel | 0x10
    regID[3]   = DAP_DP_REG_APSEL;
    regData[3] = AP_Sel | 0x10;

    // R/W DAP Registers
    status = rddi::DAP_RegAccessBlock(rddi::k_rddi_handle, 0, 4, regID, regData);
    status = SWD_CheckStatus(status);
    if (status)
        goto fail;

    AP_Bank = 0x10;

    // Select register R0 to read (write to DCRSR)
    regID[0]   = DAP_REG_AP_0x4;
    regData[0] = 0;
    // Read and wait for register ready flag (read DHCSR.16)
    regID[1]   = DAP_REG_AP_0x0 | DAP_REG_RnW | DAP_REG_WaitForValue;
    regData[1] = 0x00010000; // Value to Match
    // Read register value (read DCRDR)
    regID[2] = DAP_REG_AP_0x8 | DAP_REG_RnW;

    // DP_CTRL_STAT read
    regID[3] = DAP_REG_DP_0x4 | DAP_REG_RnW;

    // R/W DAP Registers
    status = rddi::DAP_RegAccessBlock(rddi::k_rddi_handle, 0, 4, regID, regData);
    status = SWD_CheckStatus(status);
    if (status)
        goto fail;

    status = SWD_CheckStickyError(regData[3]);
    if (status)
        goto fail;


    *rval = regData[2];

fail:
    if (status == rddi::RDDI_DAP_ERROR_MEMORY) {
        return EU14;
    } else if (status != 0) {
        return EU01;
    }

    return (0);
}


// SWD Init Debugger
//   return value: error status
int SWD_DebugInit(void)
{
    int   status;
    DWORD tick;
    DWORD val;

    if ((SWD_IDCode & 1) && ((SWD_IDCode & DPID_DESIGN_M) == (DPID_DESIGNER << DPID_DESIGN_P))) {
        DP_Ver = (BYTE)((SWD_IDCode & DPID_VER_M) >> DPID_VER_P);
        DP_Min = (SWD_IDCode & DPID_MIN) ? TRUE : FALSE;
    }

    status = SWD_WriteDP(DP_ABORT, STKCMPCLR | STKERRCLR | WDERRCLR | ORUNERRCLR);
    if (status)
        return (status);

    status = SWD_WriteDP(DP_SELECT, AP_Sel);
    if (status)
        return (status);

    status = SWD_WriteDP(DP_CTRL_STAT, CDBGPWRUPREQ | CSYSPWRUPREQ);
    if (status)
        return (status);

    tick = GetTickCount();
    do {
        status = SWD_ReadDP(DP_CTRL_STAT, &val);
        if (status)
            return (status);
        if ((val & (CDBGPWRUPACK | CSYSPWRUPACK)) == (CDBGPWRUPACK | CSYSPWRUPACK))
            break;
    } while ((GetTickCount() - tick) < 1000);

    if ((val & (CDBGPWRUPACK | CSYSPWRUPACK)) != (CDBGPWRUPACK | CSYSPWRUPACK)) {
        return (EU11); // Device could not be powered up
    }

// Removed usage of DP CTRL/STAT CDBGRSTREQ bit. It is a last resort solution to recover from a locked up device
// and should not be used as part of each connection.
#if 0
  status = SWD_WriteDP(DP_CTRL_STAT, CDBGPWRUPREQ|CSYSPWRUPREQ|CDBGRSTREQ);
  if (status) return (status);
#endif

    status = SWD_WriteDP(DP_CTRL_STAT, CDBGPWRUPREQ | CSYSPWRUPREQ | TRNNORMAL | MASKLANE);
    if (status)
        return (status);

    // Clear DP error bits
    status = SWD_WriteDP(DP_ABORT, STKCMPCLR | STKERRCLR | WDERRCLR | ORUNERRCLR);
    if (status)
        return (status);

    JTAG_devs.icacc[JTAG_devs.com_no] = 1; // set access marker for power-down on disconnect

    return (0);
}


// SWD Test Sizes Supported in AP CSW
//   return value: error status
int SWD_TestSizesAP(void)
{
    int         status = 0;
    DWORD       val;
    AP_CONTEXT *apCtx;

    // 27.06.2019: Updated AP handling
    status = AP_CurrentCtx(&apCtx);
    if (status)
        return (status);

    //  Test support for 8-bit accesses & packed transfer support
    // status = SWD_WriteAP(AP_CSW, (CSW_Val & (~(CSW_SIZE|CSW_ADDRINC))) | (CSW_SIZE8|CSW_PADDRINC));
    status = SWD_WriteAP(AP_CSW, (apCtx->CSW_Val_Base | CSW_SIZE8 | CSW_PADDRINC));
    if (status)
        return (status);

    status = SWD_ReadAP(AP_CSW, &val);
    if (status)
        return (status);

    if ((val & CSW_SIZE) == CSW_SIZE8) {
        // AP_AccSizes |= AP_ACCSZ_BYTE;
        apCtx->AccSizes |= AP_ACCSZ_BYTE;
    }
    if ((val & CSW_ADDRINC) == CSW_NADDRINC) {
        // AP_PT   = FALSE;
        apCtx->PT = FALSE;
    }

    // Test support for 16-bit accesses & packed transfer support
    // status = SWD_WriteAP(AP_CSW, (CSW_Val & (~(CSW_SIZE|CSW_ADDRINC))) | (CSW_SIZE16|CSW_PADDRINC));
    status = SWD_WriteAP(AP_CSW, (apCtx->CSW_Val_Base | CSW_SIZE16 | CSW_PADDRINC));
    if (status)
        return (status);

    status = SWD_ReadAP(AP_CSW, &val);
    if (status)
        return (status);

    if ((val & CSW_SIZE) == CSW_SIZE16) {
        // AP_AccSizes |= AP_ACCSZ_HWORD;
        apCtx->AccSizes |= AP_ACCSZ_HWORD;
    }
    if ((val & CSW_ADDRINC) == CSW_NADDRINC) {
        // AP_PT   = FALSE;
        apCtx->PT = FALSE;
    }

    return (status);
}

void InitSWD()
{
    SWD_IDCode = 0; // SWD ID Code
}


// SWD Data/Access Port Abort with value to write
//   val         : Value to write into ABORT register
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
int SWD_DAPAbortVal(DWORD val)
{
    int status;

    // Write Abort Register
    status = rddi::DAP_WriteReg(rddi::k_rddi_handle, 0, DAP_REG_DP_ABORT, val);
    if (status)
        return (rddi::RDDI_DAP_ERROR_DEBUG);

    return (0);
}


// SWD Read Data Block (8-bit Elements inside R/W Page Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of 8-bit Elements
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
int SWD_ReadBlockD8(DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib)
{
    int         status = 0;
    AP_CONTEXT *apCtx;
    DWORD       rwpage;

#if DBGCM_DBG_DESCRIPTION || DBGCM_DS_MONITOR
    int pstatus = 0;
#endif // DBGCM_DBG_DESCRIPTION || DBGCM_DS_MONITOR

#if DBGCM_DS_MONITOR
    BOOL dhcsr = FALSE;
#endif // DBGCM_DS_MONITOR

    if (nMany == 0)
        return (EU01);
    if (!(attrib & BLOCK_NADDRINC)) {
        if (nMany & 0x03)
            return (EU01);
        // if (nMany > RWPage) return (EU01);
    }

    // 27.06.2019: Moved further down due to changed AP handling
    // if (!(AP_AccSizes & AP_ACCSZ_BYTE)) return (EU21);   // Unsupported Memory Access Size

#if DBGCM_DS_MONITOR
    if (adr <= DBG_HCSR && (adr + nMany) > DBG_HCSR) {
        dhcsr  = TRUE;
        status = DSM_SuspendMonitor();
        if (status)
            return (status);
    }
#endif // DBGCM_DS_MONITOR

    if (!(attrib & BLOCK_NADDRINC)) {
        rwpage = AP_CurrentRWPage(); // Get effective RWPage based on DP/AP selection
        if (nMany > rwpage) {
            status = (EU01);
            goto end;
        }
    }

    // 27.06.2019: Updated AP handling
    status = AP_Switch(&apCtx);
    if (status)
        goto end;

    if (!(apCtx->AccSizes & AP_ACCSZ_BYTE)) {
        status = (EU21);
        goto end;
    } // Unsupported Memory Access Size

#if DBGCM_V8M
    status = SWD_UpdateDSCSR(adr, nMany, attrib);
    if (status)
        goto end;

    status = _UpdateAPSecAttr(attrib);
    if (status)
        goto end;
#endif // DBGCM_V8M

    //...
    DEVELOP_MSG("Todo: \nImplement Function: int SWD_ReadBlockD8 (DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib), required for\n - DBGCM_MEMACCX Feature");
    // See "Setting up target memory accesses based on AP_Context" above in this file for how
    // to construct the AP CSW value to write.

    // Extend error message with details if memory access failed
    // Ideally use the actual address of the failing access
    if (status == EU14)
        SetStatusMem(EU14, adr, STATUS_MEMREAD, 1);
    if (status)
        goto end;

#if DBGCM_DBG_DESCRIPTION
    if (PDSCDebug_IsEnabled()) {
        pstatus = PDSCDebug_PatchData((U32)ACCESS_MEM, 1, adr, nMany, pB, attrib);
        if (pstatus == 0) { // Error handling for data patches...
            status = 0;     // ... if data patched then clear previous access error.
        } else if (pstatus != EU38 && status == 0) {
            status = pstatus; // ... if error during patch and no previous error.
        }
        if (status)
            goto end;
    }
#endif // DBGCM_DBG_DESCRIPTION

#if DBGCM_DS_MONITOR
    if (dhcsr) {
        DSM_ExternalDHCSR(*((DWORD *)(pB + (DBG_HCSR - adr))));
    }
#endif // DBGCM_DS_MONITOR

// #if DBGCM_DBG_DESCRIPTION || DBGCM_V8M
end:
    // #endif // DBGCM_DBG_DESCRIPTION || DBGCM_V8M

#if DBGCM_DS_MONITOR
    if (dhcsr) {
        if (status) {
            DSM_ResumeMonitor();
        } else {
            status = DSM_ResumeMonitor();
        }
    }
#endif // DBGCM_DS_MONITOR

    if (status)
        return (status);

    return (0);
}


// SWD Read Data Block (16-bit Elements inside R/W Page Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of 16-bit Elements
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
int SWD_ReadBlockD16(DWORD adr, U16 *pB, DWORD nMany, BYTE attrib)
{
    int         status = 0;
    AP_CONTEXT *apCtx;
    DWORD       rwpage;

#if DBGCM_DBG_DESCRIPTION || DBGCM_DS_MONITOR
    int pstatus = 0;
#endif // DBGCM_DBG_DESCRIPTION || DBGCM_DS_MONITOR

#if DBGCM_DS_MONITOR
    BOOL dhcsr = FALSE;
#endif // DBGCM_DS_MONITOR

    if (nMany == 0)
        return (EU01);
    if (!(attrib & BLOCK_NADDRINC)) {
        if (nMany & 0x01)
            return (EU01);
        // if ((nMany*2) > RWPage) return (EU01);
    }

    // 27.06.2019: Moved further down due to changed AP handling
    // if (!(AP_AccSizes & AP_ACCSZ_HWORD)) return (EU21);   // Unsupported Memory Access Size

#if DBGCM_DS_MONITOR
    if (adr <= DBG_HCSR && (adr + (nMany << 1)) > DBG_HCSR) {
        dhcsr  = TRUE;
        status = DSM_SuspendMonitor();
        if (status)
            return (status);
    }
#endif // DBGCM_DS_MONITOR

    if (!(attrib & BLOCK_NADDRINC)) {
        rwpage = AP_CurrentRWPage(); // Get effective RWPage based on DP/AP selection
        if ((nMany * 2) > rwpage) {
            status = (EU01);
            goto end;
        }
    }

    // 27.06.2019: Updated AP handling
    status = AP_Switch(&apCtx);
    if (status)
        goto end;

    if (!(apCtx->AccSizes & AP_ACCSZ_HWORD)) {
        status = (EU21);
        goto end;
    } // Unsupported Memory Access Size

#if DBGCM_V8M
    status = SWD_UpdateDSCSR(adr, nMany << 1, attrib);
    if (status)
        goto end;

    status = _UpdateAPSecAttr(attrib);
    if (status)
        goto end;

#endif // DBGCM_V8M

    //...
    DEVELOP_MSG("Todo: \nImplement Function: int SWD_ReadBlockD16 (DWORD adr, U16 *pB, DWORD nMany, BYTE attrib), required for\n - DBGCM_MEMACCX Feature");
    // See "Setting up target memory accesses based on AP_Context" above in this file for how
    // to construct the AP CSW value to write.

    // Extend error message with details if memory access failed
    // Ideally use the actual address of the failing access
    if (status == EU14)
        SetStatusMem(EU14, adr, STATUS_MEMREAD, 2);
    if (status)
        goto end;

#if DBGCM_DBG_DESCRIPTION
    if (PDSCDebug_IsEnabled()) {
        pstatus = PDSCDebug_PatchData((U32)ACCESS_MEM, 2, adr, (nMany * 2), (UC8 *)pB, attrib);
        if (pstatus == 0) { // Error handling for data patches...
            status = 0;     // ... if data patched then clear previous access error.
        } else if (pstatus != EU38 && status == 0) {
            status = pstatus; // ... if error during patch and no previous error.
        }
        if (status)
            goto end;
    }
#endif // DBGCM_DBG_DESCRIPTION

#if DBGCM_DS_MONITOR
    if (dhcsr) {
        DSM_ExternalDHCSR(*((DWORD *)(pB + ((DBG_HCSR - adr) >> 1))));
    }
#endif // DBGCM_DS_MONITOR

// #if DBGCM_DBG_DESCRIPTION || DBGCM_V8M
end:
    // #endif // DBGCM_DBG_DESCRIPTION || DBGCM_V8M

#if DBGCM_DS_MONITOR
    if (dhcsr) {
        if (status) {
            DSM_ResumeMonitor();
        } else {
            status = DSM_ResumeMonitor();
        }
    }
#endif // DBGCM_DS_MONITOR

    if (status)
        return (status);

    return (0);
}


// SWD Read Data Block (32-bit Elements inside R/W Page Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of 32-bit Elements
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
int SWD_ReadBlockD32(DWORD adr, U32 *pB, DWORD nMany, BYTE attrib)
{
    int         status = 0;
    AP_CONTEXT *apCtx;
    DWORD       rwpage;

#if DBGCM_DBG_DESCRIPTION || DBGCM_DS_MONITOR
    int pstatus = 0;
#endif // DBGCM_DBG_DESCRIPTION || DBGCM_DS_MONITOR

#if DBGCM_DS_MONITOR
    BOOL dhcsr = FALSE;
#endif // DBGCM_DS_MONITOR

    if (nMany == 0)
        return (EU01);
        // if (((attrib & BLOCK_NADDRINC) == 0) && (nMany*4) > RWPage) return (EU01);

#if DBGCM_DS_MONITOR
    if (adr <= DBG_HCSR && (adr + (nMany << 2)) > DBG_HCSR) {
        dhcsr  = TRUE;
        status = DSM_SuspendMonitor();
        if (status)
            return (status);
    }
#endif // DBGCM_DS_MONITOR

    if (!(attrib & BLOCK_NADDRINC)) {
        rwpage = AP_CurrentRWPage(); // Get effective RWPage based on DP/AP selection
        if ((nMany * 4) > rwpage) {
            status = (EU01);
            goto end;
        }
    }

    // 27.06.2019: Updated AP handling
    status = AP_Switch(&apCtx);
    if (status)
        goto end;

#if DBGCM_V8M
    status = SWD_UpdateDSCSR(adr, nMany << 2, attrib);
    if (status)
        goto end;

    status = _UpdateAPSecAttr(attrib);
    if (status)
        goto end;
#endif // DBGCM_V8M

    //...
    DEVELOP_MSG("Todo: \nImplement Function: int SWD_ReadBlockD32 (DWORD adr, U32 *pB, DWORD nMany, BYTE attrib), required for\n - DBGCM_MEMACCX Feature");
    // See "Setting up target memory accesses based on AP_Context" above in this file for how
    // to construct the AP CSW value to write.

    // Extend error message with details if memory access failed
    // Ideally use the actual address of the failing access
    if (status == EU14)
        SetStatusMem(EU14, adr, STATUS_MEMREAD, 4);
    if (status)
        goto end;

#if DBGCM_DBG_DESCRIPTION
    if (PDSCDebug_IsEnabled()) {
        pstatus = PDSCDebug_PatchData((U32)ACCESS_MEM, 4, adr, (nMany * 4), (UC8 *)pB, attrib);
        if (pstatus == 0) { // Error handling for data patches...
            status = 0;     // ... if data patched then clear previous access error.
        } else if (pstatus != EU38 && status == 0) {
            status = pstatus; // ... if error during patch and no previous error.
        }
        if (status)
            goto end;
    }
#endif // DBGCM_DBG_DESCRIPTION

#if DBGCM_DS_MONITOR
    if (dhcsr) {
        DSM_ExternalDHCSR(*((DWORD *)(pB + ((DBG_HCSR - adr) >> 2))));
    }
#endif // DBGCM_DS_MONITOR

// #if DBGCM_DBG_DESCRIPTION || DBGCM_V8M
end:
    // #endif // DBGCM_DBG_DESCRIPTION || DBGCM_V8M

#if DBGCM_DS_MONITOR
    if (dhcsr) {
        if (status) {
            DSM_ResumeMonitor();
        } else {
            status = DSM_ResumeMonitor();
        }
    }
#endif // DBGCM_DS_MONITOR
    if (status)
        return (status);

    return (0);
}


// SWD Write Data Block (8-bit Elements inside R/W Page Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of 8-bit Elements
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
int SWD_WriteBlockD8(DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib)
{
    AP_CONTEXT *apCtx;
    DWORD       rwpage;

    // #if DBGCM_V8M
    int status = 0;
    // #endif // DBGCM_V8M

    if (nMany == 0)
        return (EU01);
    if (!(attrib & BLOCK_NADDRINC)) {
        if (nMany & 0x03)
            return (EU01);
        // if (nMany > RWPage) return (EU01);
    }

    if (!(attrib & BLOCK_NADDRINC)) {
        rwpage = AP_CurrentRWPage(); // Get effective RWPage based on DP/AP selection
        if (nMany > rwpage)
            return (EU01);
    }

    // 27.06.2019: Updated AP handling
    status = AP_Switch(&apCtx);
    if (status)
        return (status);

    // if (!(AP_AccSizes & AP_ACCSZ_BYTE)) return (EU21);   // Unsupported Memory Access Size
    if (!(apCtx->AccSizes & AP_ACCSZ_BYTE))
        return (EU21); // Unsupported Memory Access Size

#if DBGCM_V8M
    status = SWD_UpdateDSCSR(adr, nMany, attrib);
    if (status)
        return (status);

    status = _UpdateAPSecAttr(attrib);
    if (status)
        return (status);
#endif // DBGCM_V8M

    //...
    DEVELOP_MSG("Todo: \nImplement Function: int SWD_WriteBlockD8 (DWORD adr, BYTE *pB, DWORD nMany, BYTE attrib), required for\n - DBGCM_MEMACCX Feature");
    // See "Setting up target memory accesses based on AP_Context" above in this file for how
    // to construct the AP CSW value to write.

    // Extend error message with details if memory access failed
    // Ideally use the actual address of the failing access
    if (status == EU14)
        SetStatusMem(EU14, adr, STATUS_MEMWRITE, 1);
    if (status)
        return (status);

    return (0);
}


// SWD Write Data Block (16-bit Elements inside R/W Page Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of 16-bit Elements
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
int SWD_WriteBlockD16(DWORD adr, U16 *pB, DWORD nMany, BYTE attrib)
{
    AP_CONTEXT *apCtx;
    DWORD       rwpage;

    // #if DBGCM_V8M
    int status = 0;
    // #endif // DBGCM_V8M

    if (nMany == 0)
        return (EU01);
    if (!(attrib & BLOCK_NADDRINC)) {
        if (nMany & 0x01)
            return (EU01);
        // if ((nMany*2) > RWPage) return (EU01);
    }

    if (!(attrib & BLOCK_NADDRINC)) {
        rwpage = AP_CurrentRWPage(); // Get effective RWPage based on DP/AP selection
        if ((nMany * 2) > rwpage)
            return (EU01);
    }

    // 27.06.2019: Updated AP handling
    status = AP_Switch(&apCtx);
    if (status)
        return (status);

    // if (!(AP_AccSizes & AP_ACCSZ_HWORD)) return (EU21);   // Unsupported Memory Access Size
    if (!(apCtx->AccSizes & AP_ACCSZ_HWORD))
        return (EU21); // Unsupported Memory Access Size

#if DBGCM_V8M
    status = SWD_UpdateDSCSR(adr, nMany << 1, attrib);
    if (status)
        return (status);

    status = _UpdateAPSecAttr(attrib);
    if (status)
        return (status);
#endif // DBGCM_V8M

    //...
    DEVELOP_MSG("Todo: \nImplement Function: int SWD_WriteBlockD16 (DWORD adr, U16 *pB, DWORD nMany, BYTE attrib), required for\n - DBGCM_MEMACCX Feature");
    // See "Setting up target memory accesses based on AP_Context" above in this file for how
    // to construct the AP CSW value to write.

    // Extend error message with details if memory access failed
    // Ideally use the actual address of the failing access
    if (status == EU14)
        SetStatusMem(EU14, adr, STATUS_MEMWRITE, 2);
    if (status)
        return (status);

    return (0);
}


// SWD Write Data Block (32-bit Elements inside R/W Page Block)
//   adr    : Address
//   pB     : Pointer to Buffer
//   nMany  : Number of 32-bit Elements
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
int SWD_WriteBlockD32(DWORD adr, U32 *pB, DWORD nMany, BYTE attrib)
{
    AP_CONTEXT *apCtx;
    DWORD       rwpage;

    // #if DBGCM_V8M
    int status = 0;
    // #endif // DBGCM_V8M

    if (nMany == 0)
        return (EU01);
    // if (((attrib & BLOCK_NADDRINC) == 0) && (nMany*4) > RWPage) return (EU01);
    if (!(attrib & BLOCK_NADDRINC)) {
        rwpage = AP_CurrentRWPage(); // Get effective RWPage based on DP/AP selection
        if ((nMany * 4) > rwpage)
            return (EU01);
    }

    // 27.06.2019: Updated AP handling
    status = AP_Switch(&apCtx);
    if (status)
        return (status);

#if DBGCM_V8M
    status = SWD_UpdateDSCSR(adr, nMany << 2, attrib);
    if (status)
        return (status);

    status = _UpdateAPSecAttr(attrib);
    if (status)
        return (status);
#endif // DBGCM_V8M

    //...
    DEVELOP_MSG("Todo: \nImplement Function: int SWD_WriteBlockD32 (DWORD adr, U32 *pB, DWORD nMany, BYTE attrib), required for\n - DBGCM_MEMACCX Feature");
    // See "Setting up target memory accesses based on AP_Context" above in this file for how
    // to construct the AP CSW value to write.

    // Extend error message with details if memory access failed
    // Ideally use the actual address of the failing access
    if (status == EU14)
        SetStatusMem(EU14, adr, STATUS_MEMWRITE, 4);
    if (status)
        return (status);

    return (0);
}


// SWD Read ARM Memory (8-bit accesses only)
//   nAdr   : Start Address (used to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of 8-bit Elements to Read
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
int SWD_ReadARMMemD8(DWORD *nAdr, BYTE *pB, DWORD nMany, BYTE attrib)
{
    return SWD_ReadARMMem(nAdr, (BYTE *)pB, nMany * 1, attrib);
}


// SWD Read ARM Memory (16-bit accesses only)
//   nAdr   : Start Address (used to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of 16-bit Elements to Read
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
int SWD_ReadARMMemD16(DWORD *nAdr, U16 *pB, DWORD nMany, BYTE attrib)
{
    return SWD_ReadARMMem(nAdr, (BYTE *)pB, nMany * 2, attrib);
}


// SWD Read ARM Memory (32-bit accesses only)
//   nAdr   : Start Address (used to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of 32-bit Elements to Read
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
int SWD_ReadARMMemD32(DWORD *nAdr, U32 *pB, DWORD nMany, BYTE attrib)
{
    return SWD_ReadARMMem(nAdr, (BYTE *)pB, nMany * 4, attrib);
}


// SWD Write ARM Memory (8-bit accesses only)
//   nAdr   : Start Address (used to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of 8-bit Elements to Write
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
int SWD_WriteARMMemD8(DWORD *nAdr, BYTE *pB, DWORD nMany, BYTE attrib)
{
    return SWD_WriteARMMem(nAdr, (BYTE *)pB, nMany * 1, attrib);
}


// SWD Write ARM Memory (16-bit accesses only)
//   nAdr   : Start Address (used to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of 16-bit Elements to Write
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
int SWD_WriteARMMemD16(DWORD *nAdr, U16 *pB, DWORD nMany, BYTE attrib)
{
    return SWD_WriteARMMem(nAdr, (BYTE *)pB, nMany * 2, attrib);
}


// SWD Write ARM Memory (32-bit accesses only)
//   nAdr   : Start Address (used to return error addresses)
//   pB     : Pointer to Buffer
//   nMany  : Number of 32-bit Elements to Write
//   attrib : Attributes for memory access (Bit 0 - NoAddrIncr, Bits [2..1] - Security Attribute)
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
int SWD_WriteARMMemD32(DWORD *nAdr, U32 *pB, DWORD nMany, BYTE attrib)
{
    return SWD_WriteARMMem(nAdr, (BYTE *)pB, nMany * 4, attrib);
}


// SWD Configure SWJ Debug Protocol
//   retry: Number of this retry
//   return  value: error status
//
// Required for:
//  - DBGCM_DBG_DESCRIPTION Feature
int SWJ_ConfigureProtocol(int retry)
{
    int  status = 0;
    bool swj    = (MonConf.Opt & USE_SWJ) != 0;

    if (swj) {
        // SWJ Switch to SW
        SWJ_SwitchSeq = (retry ? 0xEDB6 : 0xE79E);

        // RDDI will handle this automatically, no need to send the sequence again
        // Implement SWJ Sequence
        //   1. 51*TMS/SWDIO HIGH (or more)
        //   2. 16-bit Switch Sequence (SWJ_SwitchSeq)
        //   3. 51*TMS/SWDIO HIGH (or more)
    } else {
        status = SWJ_Reset();
        if (status)
            return (status);
    }

    status = SWD_ReadID();
    if (status)
        return (status);

    return (0);
}


// SWD Read Device List
//   return  value: error status
//
// Required for:
//  - DBGCM_DBG_DESCRIPTION Feature
int SWD_GetDeviceList(JDEVS *DevList, unsigned int maxdevs, bool merge)
{
    int  status  = 0;
    bool portSet = (merge && DevList->cnt > 0);

    if (!DevList || !maxdevs)
        return (EU39);

    // Init if no port info from PDSC
    if (!portSet) {
        DevList->cnt    = 0;
        DevList->com_no = 0;
    }

    // Do the read regardless of "merge" to enable the debug port
    status = SWD_ReadID();
    if (status)
        return (status);

    if (!merge || DevList->ic[0].id == 0) {
        // No PDSC info or no ID provided => no override from PDSC available
        if ((SWD_IDCode & 1) == 0) {
            status = EU10;
            return (status);
        }
    }

    if (!merge || DevList->ic[0].id == 0) {
        // No PDSC port description or no ID Code provided
        DevList->ic[0].id = SWD_IDCode;
    }
    DevList->ic[0].ir_len = 0; // IR Length not applicable to SW-DP
    DevList->cnt          = 1; // Keep as is as long as there is no multi-drop support

    return (0);
}


// SWD Get Device Names of ICs connected to the SW-DP
//
// Required for:
//  - DBGCM_DBG_DESCRIPTION Feature
int SWD_GetDeviceNames(JDEVS *DevList, unsigned int maxdevs, bool merge)
{
    int status;
    int setNameLen = strlen(DevList->icname[0]);
    int maxNameLen = sizeof(DevList->icname[0]) - 1;

    if (!DevList || !maxdevs)
        return (EU39);

    nCPU   = -1;
    status = 0;

    // Check for known SW-DP ID
    switch (DevList->ic[0].id & 0x0FF1FFFF) {
        case 0x0BA01477: // DP V1
        case 0x0BA02477: // DP V2
        case 0x0BA11477: // DP V1 MinDP
        case 0x0BA12477: // DP V2 MinDP
        case 0x0BB11477: // DP V1 MinDP (ARM Cortex-M0)
        case 0x0BC11477: // DP V1 MinDP (ARM Cortex-M0+)
        case 0x0BC12477: // DP V2 MinDP (ARM Cortex-M0+ with Multi-Drop SW)
        case 0x0BD11477: // DP V1 MinDP (ARM Cortex-M7)
        case 0x0BD12477: // DP V2 MinDP (ARM Cortex-M7 with Multi-Drop SW)
        case 0x0BE12477: // DP V2 MinDP (ARM Cortex-M33)
        case 0x0BF11477: // DP V1 MinDP (ARM Cortex-M23)
        case 0x0BF12477: // DP V2 MinDP (ARM Cortex-M23 with Multi-Drop SW)

            nCPU = 0;

            MonConf.JtagCpuIndex = 0; // Set DP selection marker

            if (merge && setNameLen > 0) {
                strncat(DevList->icname[0], " (ARM CoreSight SW-DP)", maxNameLen - setNameLen);
                DevList->icname[0][maxNameLen] = '\0'; // maxNameLen excludes terminating '\0'
            } else {
                strcpy(DevList->icname[0], "ARM CoreSight SW-DP");
            }
            DevList->icinfo[0] = ARMCSDP;
            break;
        default:
            if (merge) {
                if (setNameLen > 0) {
                    strncat(DevList->icname[0], " (Unknown device)", maxNameLen - setNameLen);
                } else {
                    strncat(DevList->icname[0], "(Unknown device)", maxNameLen - setNameLen);
                }
                DevList->icname[0][maxNameLen] = '\0'; // maxNameLen excludes terminating '\0'
            } else {
                strcpy(DevList->icname[0], "Unknown device");
            }
            status = EU10;
    }

    if (SetupMode)
        return (0); // Called from Setup Dialog
    return (status);
}


// SWD Power Up Debug Port
//   return  value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
int SWD_DebugPortStart(void)
{
    DWORD tick, val;
    int   status = 0;

    status = SWD_WriteDP(DP_CTRL_STAT, CDBGPWRUPREQ | CSYSPWRUPREQ);
    if (status)
        return (status);

    tick = GetTickCount();
    do {
        status = SWD_ReadDP(DP_CTRL_STAT, &val);
        if (status)
            return (status);
        if ((val & (CDBGPWRUPACK | CSYSPWRUPACK)) == (CDBGPWRUPACK | CSYSPWRUPACK))
            break;
    } while ((GetTickCount() - tick) < 1000);

    if ((val & (CDBGPWRUPACK | CSYSPWRUPACK)) != (CDBGPWRUPACK | CSYSPWRUPACK)) {
        status = EU11; // Device could not be powered up
        return (status);
    }

// Removed usage of DP CTRL/STAT CDBGRSTREQ bit. It is a last resort solution to recover from a locked up device
// and should not be used as part of each connection.
#if 0
  status = SWD_WriteDP(DP_CTRL_STAT, CDBGPWRUPREQ|CSYSPWRUPREQ|CDBGRSTREQ);
  if (status) return (status);
#endif

    status = SWD_WriteDP(DP_CTRL_STAT, CDBGPWRUPREQ | CSYSPWRUPREQ | TRNNORMAL | MASKLANE);
    if (status)
        return (status);

    status = SWD_WriteDP(DP_ABORT, STKCMPCLR | STKERRCLR | WDERRCLR | ORUNERRCLR);
    if (status)
        return (status);

    return (status);
}


// SWD Switch DP
//   id     : DP ID to switch to
//   return value: error status
//
// Required for:
//  - DBGCM_MEMACCX Feature
int SWD_SwitchDP(DWORD id, bool force)
{
    int         status = 0;
    bool        shared = false;
    AP_CONTEXT *apCtx;

    if (id == JTAG_devs.com_no && !force) {
        // Debug Port already selected
        return (0);
    }

#if DBGCM_DBG_DESCRIPTION
    if (!PDSCDebug_IsEnabled() || PDSCDebug_DevicesScanned()) {
        if (id >= JTAG_devs.cnt) {
            return (EU25); // PDSC: Unknown Debug Port ID.\nCannot switch to Debug Port.
        }
    }

    JTAG_devs.com_no = id;

    if (PDSCDebug_IsSupported()) {
        // Store info about selected DP
        status = PDSCDebug_SetActiveDP(id);
        if (status)
            return (status);
    }

    // First time switch to DP, suppress implicit DP switch + power-up for Debug Description
    if (!PDSCDebug_IsEnabled() || force) {
        JTAG_devs.icacc[JTAG_devs.com_no] = 1; // set access marker for power-down on disconnect

        if (PDSCDebug_IsEnabled()) {
            status = PDSCDebug_DebugPortStart();
            if (status)
                return (status);
        } else {
            // Power-up DP
            status = SWD_DebugPortStart();
            if (status)
                return (status);
        }

        // Init DP_SELECT
        status = SWD_WriteDP(DP_SELECT, AP_Sel); // Assuming that the desired AP has been selected by caller
        if (status)
            return (status);

#if 0 // 27.06.2019: Changed AP handling
    // Init AP CSW with current value
    status = SWD_WriteAP(AP_CSW, CSW_Val);
    if (status) return (status);
#endif

        // Init AP CSW
        status = AP_Switch(&apCtx);
        if (status)
            return (status);

        status = SWD_WriteAP(AP_CSW, (apCtx->CSW_Val_Base | CSW_SIZE32 | CSW_SADDRINC));
        if (status)
            return (status);
    }
#else // DBGCM_DBG_DESCRIPTION
    JTAG_devs.com_no = id;

    JTAG_devs.icacc[JTAG_devs.com_no] = 1; // set access marker for power-down on disconnect

    // Power-up DP
    status = SWD_DebugPortStart();
    if (status)
        return (status);

    // Init DP_SELECT
    status = SWD_WriteDP(DP_SELECT, AP_Sel); // Assuming that the desired AP has been selected by caller
    if (status)
        return (status);

#if 0 // 27.06.2019: Changed AP handling
  // Init AP CSW with current value
  status = SWD_WriteAP(AP_CSW, CSW_Val);
  if (status) return (status);
#endif

    // Init AP CSW
    status = AP_Switch(&apCtx);
    if (status)
        return (status);

    status = SWD_WriteAP(AP_CSW, (apCtx->CSW_Val_Base | CSW_SIZE32 | CSW_SADDRINC));
    if (status)
        return (status);

#endif // DBGCM_DBG_DESCRIPTION

    return (0);
}


// SWD Execute SWJ Sequence
//   cnt    : Length of sequence in bits (0 < cnt <= 64)
//   val    : TMS values, LSB first
//   return  value: error status
//
// Required for:
//  - DBGCM_DBG_DESCRIPTION Feature
//  - DBGCM_RECOVERY Feature
int SWD_SWJ_Sequence(int cnt, U64 val)
{
    //...
    DEVELOP_MSG("Todo: \nImplement Function: int SWD_SWJ_Sequence (int cnt, U64 val), required for\n - DBGCM_DBG_DESCRIPTION Feature\n - DBGCM_RECOVERY Feature");
    return (0);
}


// SWD Set Debugger Clock
//   cid    : Debugger Specific Clock ID
//   rtck   : Use Return Clock (FALSE for Cortex-M)
//   return  value: error status
//
// Required for:
//  - DBGCM_DBG_DESCRIPTION Feature
int SWD_SWJ_Clock(BYTE cid, BOOL rtck)
{
    //...
    DEVELOP_MSG("Todo: \nImplement Function: int SWD_SWJ_Clock (BYTE cid, BOOL rtck), required for\n - DBGCM_DBG_DESCRIPTION Feature");
    return (0);
}

#if DBGCM_V8M
// Update DSCSR Secured Bank Register Selection
//   adr   : address to be accessed
//   many   : number of bytes to be accessed
//   attrib : memory access attribute
//   return value: error status
static int SWD_UpdateDSCSR(DWORD adr, DWORD many, BYTE attrib)
{
    int   status = 0;
    DWORD sbrsel = 0, dscsr = 0;

#if DBGCM_DBG_DESCRIPTION
    int pstatus = 0;
#endif // DBGCM_DBG_DESCRIPTION

    if (!pio->bSecureExt)
        return (0); // No Security Extensions
    if (!OverlapSCSv8M(adr, many))
        return (0); // No need to update
    if ((attrib & BLOCK_SECTYPE) == BLOCK_SECTYPE_ANY)
        return (0); // Keep current SBRSEL/SBRSELEN selection

    // Read Current DSCSR value

    //---TODO:
    //...
    DEVELOP_MSG("Todo: \nImplement Function: int SWD_UpdateDSCSR (DWORD adr, DWORD many, BYTE attrib) - Read DSCSR, required for\n - DBGCM_V8M Feature");

#if DBGCM_DBG_DESCRIPTION
    // Patch if necessary
    if (PDSCDebug_IsEnabled()) {
        pstatus = PDSCDebug_PatchData((U32)ACCESS_MEM, 4, DBG_SCSR, 4, (UC8 *)dscsr, attrib);
        if (pstatus == 0) { // Error handling for data patches...
            status = 0;     // ... if data patched then clear previous access error.
        } else if (pstatus != EU38 && status == 0) {
            status = pstatus; // ... if error during patch and no previous error.
        }
    }
    if (status)
        return (status);
#endif // DBGCM_DBG_DESCRIPTION

    // Update and Write DSCSR value
    sbrsel = 0;
    if ((attrib & BLOCK_SECTYPE) != BLOCK_SECTYPE_CPU) { // Specific security attribute selected
        sbrsel = SBRSELEN | ((attrib & BLOCK_SECTYPE_SECURE) ? SBRSEL : 0);
    }

    if ((dscsr & (SBRSELEN | SBRSEL)) != sbrsel) {
        if (!DSCSR_Has_CDSKEY) {
            if (iRun || iBpCmd)
                return (EU47); // Cannot change security view while target is executing);
        }
        dscsr &= ~(SBRSELEN | SBRSEL);
        dscsr |= sbrsel;
        dscsr |= CDSKEY; // Ensure CDS is locked, RAZ/WI if not implemented, old CDS value preserved

        //---TODO:
        //...
        DEVELOP_MSG("Todo: \nImplement Function: int JTAG_UpdateDSCSR (DWORD adr, DWORD many, BYTE attrib) - Write updated DSCSR, required for\n - DBGCM_V8M Feature");
    }

    return (0);
}
#endif // DBGCM_V8M
