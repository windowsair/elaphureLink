/**************************************************************************/ /**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.0.6
 * @date     $Date: 2016-07-18 13:20:49 +0200 (Mon, 18 Jul 2016) $
 *
 * @note
 * Copyright (C) 2016, 2019-2020 ARM Limited. All rights reserved.
 *
 * @brief     Breakpoint Resource Manager
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

#include "stdafx.h"

#include "BreakResources.h"
#include "..\TracePointDefs.h"
#include "..\BOM.H"
#include "..\ALLOC.H"
#include "COLLECT.H"
#include "Debug.h"
#include "..\AGDI.H"

extern int     SetClrSwBP(DWORD set, AG_BP *pB);
extern AG_BP **pBhead; // Defined in AGDI.cpp

// Defines
#define MAX_ETM_TRIG_IN 2


// Resource Info Instances
BREAK_RESOURCES     BreakResources;
DWT_RESOURCES       DwtResources;
SW_BREAK_RESOURCES  SwBreakResources;
ETM_RESOURCES       EtmResources;
RUN_BREAK_RESOURCES RunBreakResources;

// Used Resources During Target Run
USED_RESOURCES UsedBreakResources;

// Alternatively search for a different condition to remove the next state variable
static bool MergingRunBreaks = false;

// SW Break Configurations
AG_SWBREAKCONF_ITEM *SWBCHead        = NULL;
AG_SWBREAKCONF_ITEM *SWBCGarbageHead = NULL;


/*
 *  DumpBreakResources():
 *    - Dumps current state of the breakpoint resource allocation.
 */
void DumpBreakResources()
{
    DWORD i;

    SW_BREAK *pCurBreak = SwBreakResources.pSwBreaks;
    TRACE("BREAK RESOURCES:\n");

    TRACE("\tFPB/Breakpoint Unit\n");
    TRACE("\t\tFull Address Space     : %s\n", BreakResources.FullAddrSpace ? "true" : "false");
    TRACE("\t\tNum Breaks             : %d\n", BreakResources.NumBreaks);
    TRACE("\t\tNum Literals           : %d\n", BreakResources.NumLiterals);
    TRACE("\t\tNum Allocated Breaks   : %d\n", BreakResources.NumBreaksAlloced);
    TRACE("\t\tNum Allocated Literals : %d\n", BreakResources.NumLiteralsAlloced);

    TRACE("\tDWT Unit\n");
    TRACE("\t\tNum Comparators                          : %d\n", DwtResources.NumComps);
    TRACE("\t\tNum Cycle Comparators                    : %d\n", DwtResources.NumCycleComps);
    TRACE("\t\tNum Value Comparators                    : %d\n", DwtResources.NumValueComps);
    if (IsV8M()) {
        TRACE("\t\tNum Limit Comparators                  : %d\n", DwtResources.NumLimitComps);
        TRACE("\t\tNum Linkable Comparators               : %d\n", DwtResources.NumLinkComps);
    } else {
        TRACE("\t\tNum 2Link Comparators                  : %d\n", DwtResources.NumLink2ndComps);
    }
    TRACE("\t\tNum Allocated Comparators                : %d\n", DwtResources.NumCompsAlloced);
    TRACE("\t\tNum Allocated Cycle Comparators          : %d\n", DwtResources.NumCycleCompsAlloced);
    TRACE("\t\tNum Allocated Value Comparators          : %d\n", DwtResources.NumValueCompsAlloced);
    if (IsV8M()) {
        TRACE("\t\tNum Allocated Limit Comparators        : %d\n", DwtResources.NumLimitCompsAlloced);
        TRACE("\t\tNum Allocated Linkable Comparators     : %d\n", DwtResources.NumLinkCompsAlloced);
    } else {
        TRACE("\t\tNum Allocated 2Link Comparators        : %d\n", DwtResources.NumLink2ndCompsAlloced);
    }
    TRACE("\t\tNum Comparators allocated as Breakpoint  : %d\n", DwtResources.NumCompsBreakAlloced);
    TRACE("\t\tMax mask bits                            : %d\n", DwtResources.MaxMaskBits);

    TRACE("\tSW Breakpoints\n");
    TRACE("\t\tSW Breakpoints Allowed              : %s\n", SwBreakResources.Allowed ? "true" : "false");
    TRACE("\t\tNum SW Breakpoints                  : %d\n", SwBreakResources.NumSWBreaks);
    for (; pCurBreak; pCurBreak = pCurBreak->pNext) {
        if (pCurBreak->bValid) {
            TRACE("\t\tSW Breakpoint Valid\n");
        } else {
            TRACE("\t\tSW Breakpoint Invalid\n");
        }
        if (pCurBreak->bCanSet) {
            TRACE("\t\tCan set SW Breakpoint at            : 0x%08X\n", pCurBreak->nAddr);
        } else {
            TRACE("\t\tCannot set SW Breakpoint at         : 0x%08X\n", pCurBreak->nAddr);
        }
    }

    TRACE("\tRun Breakpoints\n");
    TRACE("\t\tMax Breakpoints Set              : %d\n", RunBreakResources.MaxRunBreaks);
    TRACE("\t\tRun Breakpoints Set              : %d\n", RunBreakResources.SetRunBreaks);
    TRACE("\t\tAdditional HW Break Comps        : %d\n", RunBreakResources.AddCompsBreakUsed);
    if (RunBreakResources.SetRunBreaks > 0) {
        for (i = 0; i < RunBreakResources.MaxRunBreaks; i++) {
            if (RunBreakResources.RunBreaks[i].bActive) {
                TRACE("\t\tRun Breakpoint Set at               : 0x%08X\n", RunBreakResources.RunBreaks[i].nAddr);
            }
        }
    }


    TRACE("\tETM\n");
    TRACE("\t\tImplemented                   ?  %s\n", (EtmResources.Capabilities & ETM_CAP_IMPLEMENTED) ? "YES" : "NO");
    TRACE("\t\tStart/Stop Logic              ?  %s\n", (EtmResources.Capabilities & ETM_CAP_START_STOP) ? "YES" : "NO");
    TRACE("\t\tDWT Input to Start/Stop Logic ?  %s\n", (EtmResources.Capabilities & ETM_CAP_START_STOP_DWT) ? "YES" : "NO");
    TRACE("\t\tNum DWT Inputs                        : %d\n", EtmResources.NumDwtInputs);
    TRACE("\t\tNum Allocated Possible Trigger Inputs : %d\n", EtmResources.NumTrigInputsAlloced);

    TRACE("\tUsed Resources During Target Run\n");
    TRACE("\t\tUsed HW Breaks       : %d\n", UsedBreakResources.NumBreaks);
    TRACE("\t\tUsed Literals        : %d\n", UsedBreakResources.NumLiterals);
}


/*
 *  BreakOnWord():
 *    - Checks memory attributes for possibly set break attributes, catches any break attribute
 */
static BOOL BreakOnWord(AG_BP *pBp, bool swbreak)
{
    DWORD *pB   = MGetAttr(pBp->Adr); // word attributes
    AG_BP *pCBp = *pBhead;

    if (BreakResources.LHSupported && !swbreak) {
        if (pB && (*pB & (ATRX_BREAK | ATRX_BREAKO))) {
            // Breakpoint on word, check if an HW Breakpoint Comparator is active for this word
            for (; pCBp != NULL; pCBp = pCBp->next) {
                if (pCBp->type == ABREAK && pCBp->enabled && pCBp->bAlloced && ((pCBp->Adr & (~0x3)) == (pBp->Adr & (~0x3)))) {
                    if (pCBp->Opc[4] == 0) { // Active Hardware Breakpoint for this word
                        return TRUE;
                    }
                }
            }
        }
    } else {
        DWORD nComp = (pBp->Adr & 0x2) ? ATRX_BREAKO : ATRX_BREAK; // compare mem attribs for address
        if (pB && (*pB & nComp)) {
            // Breakpoint on exact address, check if an active breakpoint uses the same breakpoint type (HW/SW)
            for (; pCBp != NULL; pCBp = pCBp->next) {
                if (pCBp->type == ABREAK && pCBp->enabled && pCBp->bAlloced && pCBp->Adr == pBp->Adr) {
                    if (swbreak) {
                        if (pCBp->Opc[4] != 0) { // Software Breakpoint already written
                            return TRUE;
                        }
                    } else {
                        if (pCBp->Opc[4] == 0) { // Hardware Breakpoint
                            return TRUE;
                        }
                    }
                }
            }
        }
    }

    return FALSE;
}


/*
 *  OtherBreakOnWord():
 *    - Checks memory attributes for possibly set break attributes
 *       - If breakpoint address is an odd address it checks for a break attrib on the even address
 *         of the memory word.
 *       - If breakpoint address is an even address it checks for a break attrib on the odd address
 *         of the memory word.
 */
static BOOL OtherBreakOnWord(AG_BP *pBp, bool swbreak)
{
    if (swbreak) {
        // never having "linked" software breakpoints
        return FALSE;
    }

    if (!BreakResources.LHSupported) {
        // never having an unrelated hardware breakpoint on same comparator
        return FALSE;
    }

    DWORD *pB = MGetAttr(pBp->Adr); // word attributes

    if (!pB) {
        return FALSE;
    }

    DWORD nComp = (pBp->Adr & 0x2) ? ATRX_BREAK : ATRX_BREAKO; // compare mem attribs for opposite address

    if (*pB & nComp) {
        return TRUE;
    }

    return FALSE;
}


static BOOL TestSwBreak(AG_BP *pBp)
{
    int nSetRes = 0;

    // Check if address is writeable with BKPT instruction
    if ((nSetRes = SetClrSwBP(1, pBp)) == 0) { // success
        // Clear the software breakpoint
        SetClrSwBP(0, pBp);
        return TRUE;
    }

    return FALSE;
}


/*
 *  CanSwBreak():
 *    - Checks if an address can be replaced by BKPT instructions
 *    - Uses SetClrSwBP() function
 *    - Creates a new SW_BREAK struct for later reference
 */
static BOOL CanSwBreak(AG_BP *pBp)
{
    if (!SwBreakResources.Allowed) {
        return FALSE;
    }

    SW_BREAK *pCurBreak = SwBreakResources.pSwBreaks;
    int       nSetRes   = 0;

    if (pBp->Adr >= 0xE0000000) {
        return FALSE;
    }

    for (; pCurBreak != NULL; pCurBreak = pCurBreak->pNext) {
        if (pCurBreak->nAddr == pBp->Adr) {
            if (pCurBreak->bValid) {
                return (pCurBreak->bCanSet != 0);
            }
            break; // Retest SW Breakpoint
        }
    }

    if (pCurBreak == NULL) {
        // Create a SW_BREAK
        pCurBreak = (SW_BREAK *)pio->GetMem(sizeof(SW_BREAK), ENV_DBM);
        memset(pCurBreak, 0, sizeof(SW_BREAK));
        pCurBreak->nAddr = pBp->Adr;
        // Insert it in the beginning of the list
        pCurBreak->pNext           = SwBreakResources.pSwBreaks;
        SwBreakResources.pSwBreaks = pCurBreak;
    }

    pCurBreak->bCanSet = TestSwBreak(pBp) ? 1 : 0;
    pCurBreak->bValid  = 1;
    return (pCurBreak->bCanSet ? TRUE : FALSE);
}

static unsigned long CheckHWBreakAddress(unsigned long nAddr)
{
    if (FPB_Ver == 0 && nAddr >= 0x20000000) {
        return RES_ERR_ADDR_NOTSUPPORTED;
    }
    return RES_OK;
}


void ClearRunBreaks()
{
    RunBreakResources.AddCompsBreakUsed = 0;
    RunBreakResources.SetRunBreaks      = 0;
    if (RunBreakResources.RunBreaks) {
        memset(&RunBreakResources.RunBreaks[0], 0, RunBreakResources.MaxRunBreaks * sizeof(RUN_BREAK));
    }
}


int MergeRunBreaks()
{
    DWORD      i;
    RUN_BREAK *pRB;
    AG_BP *    pBp;
    int        status, result;

    if (RunBreakResources.RunBreaks == NULL || RunBreakResources.SetRunBreaks == 0) {
        return (RES_OK);
    }

    if (pBhead == NULL) {
        return (RES_ERR_INTERNAL);
    }

    MergingRunBreaks = true; // Skip "iRun" specific handling in AllocBreakResources()
    result           = RES_OK;
    for (i = 0, pRB = RunBreakResources.RunBreaks; i < RunBreakResources.MaxRunBreaks; i++, pRB++) {
        if (pRB->bActive) {
            for (pBp = *pBhead; pBp != NULL; pBp = pBp->next) {
                if (pBp->Adr == pRB->nAddr && !pBp->bAlloced) {
                    // Now allocate the resource properly
                    status = AllocBreakResources(1, pBp);
                    if (status != RES_OK && result == RES_OK) {
                        result = status;
                    }
                }
            }
        }
    }
    MergingRunBreaks = false;

    ClearRunBreaks();

    return (result);
}


static RUN_BREAK *GetRunBreak(DWORD nAddr)
{
    DWORD      i;
    RUN_BREAK *pRB;

    if (RunBreakResources.SetRunBreaks == 0) {
        return NULL;
    }

    for (i = 0, pRB = RunBreakResources.RunBreaks; i < RunBreakResources.MaxRunBreaks && pRB != NULL; i++, pRB++) {
        if (pRB->bActive && pRB->nAddr == nAddr) {
            return pRB;
        }
    }

    return NULL;
}


static int SetRunBreak(DWORD nAddr, bool newcomp, bool runcomp)
{
    DWORD      i;
    RUN_BREAK *pRB;

    if (RunBreakResources.RunBreaks == NULL) {
        return RES_ERR_INTERNAL; // Add error code
    }

    if (newcomp && RunBreakResources.AddCompsBreakUsed + BreakResources.NumBreaksAlloced >= BreakResources.NumBreaks) {
        return RES_ERR_USED_BREAK; // All HW Breaks in use
    }

    for (i = 0, pRB = RunBreakResources.RunBreaks; i < RunBreakResources.MaxRunBreaks; i++, pRB++) {
        if (pRB->bActive == 0) {
            pRB->bActive  = 1;
            pRB->nAddr    = nAddr;
            pRB->bRunComp = runcomp ? 1 : 0;
            RunBreakResources.SetRunBreaks++;
            if (newcomp)
                RunBreakResources.AddCompsBreakUsed++;

            return (RES_OK);
        }
    }

    return (RES_ERR_USED_BREAK);
}


/*
 *  AddRunBreak():
 *    - Add RUN_BREAK, only called while running
 */
static int AddRunBreak(DWORD nAddr)
{
    int        status = RES_OK;
    RUN_BREAK *pRB;
    AG_BP *    pBp;

    // Generally possible to set this HW Breakpoint?
    status = CheckHWBreakAddress(nAddr);
    if (status != RES_OK)
        return (status);

    // Check if Run Break already exists
    pRB = GetRunBreak(nAddr);
    if (pRB && pRB->bActive)
        return (RES_OK); // Already allocated, nothing else to do

    // Check if Run Break on other half word exists (must be a HW comparator)
    if (BreakResources.LHSupported) {
        pRB = GetRunBreak((nAddr & 0x2) ? (nAddr & (~0x3)) : (nAddr | 0x2));
        if (pRB && pRB->bActive) {
            // HW Comp allocated on other half-word, add this request as a run break
            return (SetRunBreak(nAddr, false, true));
        }
    }

    // Does breakpoint on this address already exist (SW or HW)?
    pBp = pBhead ? *pBhead : NULL;
    if (BreakResources.LHSupported) {
        for (; pBp != NULL; pBp = pBp->next) {
            if (pBp->bAlloced) {         // only care about those allocated before the target ran
                if (pBp->Adr == nAddr) { // very same breakpoint already exists
                    return (RES_OK);
                } else if (pBp->Opc[4] == 0) { // HW Breakpoint
                    if ((pBp->Adr & (~0x3)) == (nAddr & (~0x3))) {
                        // HW Breakpoint on other half word, no new comparator
                        if (GetRunBreak((nAddr & 0x2) ? (nAddr & (~0x3)) : (nAddr | 0x2))) {
                            // Run Break on other halfword
                            return SetRunBreak(nAddr, false, true);
                        } else {
                            // "Regular" Break on other halfword, mark as non-runcomp
                            return SetRunBreak(nAddr, false, false);
                        }
                    }
                }
            }
        }
    } else {
        for (; pBp != NULL; pBp = pBp->next) {
            if (pBp->Adr == nAddr)
                return (RES_OK); // very same breakpoint already exists
        }
    }

    // Nothing compatbile found, add the breakpoint but check first if we've got enough resources left
    if (UsedBreakResources.NumBreaks >= BreakResources.NumBreaks) {
        return RES_ERR_USED_BREAK;
    }
    return (SetRunBreak(nAddr, true, true));
}

/*
 *  FreeRunBreak()
 *    - Free RUN_BREAK, only called while running
 */
static int FreeRunBreak(DWORD nAddr)
{
    RUN_BREAK *pRB = GetRunBreak(nAddr);

    if (pRB == NULL) {
        return RES_ERR_NO_ALLOC;
    }

    if (RunBreakResources.SetRunBreaks > 0)
        RunBreakResources.SetRunBreaks--;
    if (pRB->bRunComp) {
        if (GetRunBreak((pRB->nAddr & 0x2) ? (pRB->nAddr & (~0x3)) : (pRB->nAddr | 0x2)) == NULL) {
            // No more breaks using this comparators
            if (RunBreakResources.AddCompsBreakUsed > 0)
                RunBreakResources.AddCompsBreakUsed--;
        }
    }
    pRB->bActive = 0;
    return RES_OK;
}


/*
 * IsRunBreak()
 *  - Check if RUN_BREAK at nAddr
 */
static int IsRunBreak(DWORD nAddr)
{
    RUN_BREAK *pRB = GetRunBreak(nAddr);
    return ((pRB && pRB->bActive) ? 1 : 0);
}


/*
 * Detect breakpoint resources (FPB).
 *  - pVer : FPB architecture version.
 * Return Value: -1 if error, else the number of hardware breakpoints.
 */
int DetectBreakResources(BYTE *pVer)
{
    BYTE  version;
    DWORD val;
    int   status;

    memset(&BreakResources, 0, sizeof(BreakResources));

    if (FPB_Addr == 0) {
        BreakResources.NumBreaks   = 0;
        BreakResources.NumLiterals = 0;
        return 0;
    }

#if DBGCM_V8M
    status = ReadD32(FPB_CTRL, &val, BLOCK_SECTYPE_ANY);
    if (status) {
        OutErrorMessage(status);
        return (-1);
    }
#else  // DBGCM_V8M
    status = ReadD32(FPB_CTRL, &val);
    if (status) {
        OutErrorMessage(status);
        return (-1);
    }
#endif // DBGCM_V8M

    version                      = BYTE((val & FPB_REV) >> FPB_REV_P);
    BreakResources.FullAddrSpace = (version > 0);
    BreakResources.LHSupported   = !BreakResources.FullAddrSpace; // only for Cortex-M7, change if a mixture exists
    BreakResources.NumBreaks     = (BYTE)((val & FPB_NUM_CODE) >> FPB_NUM_CODE_P);
    BreakResources.NumLiterals   = (BYTE)((val & FPB_NUM_LIT) >> FPB_NUM_LIT_P);

    if (pVer) {
        *pVer = version;
    }

    memset(&RunBreakResources, 0, sizeof(RunBreakResources));
    RunBreakResources.MaxRunBreaks = BreakResources.NumBreaks;
    if (BreakResources.LHSupported)
        RunBreakResources.MaxRunBreaks *= 2;
    RunBreakResources.RunBreaks = (RUN_BREAK *)pio->GetMem(RunBreakResources.MaxRunBreaks * sizeof(RUN_BREAK), ENV_DBM);

    return BreakResources.NumBreaks;
}


/*
 * Detect DWT resources.
 * Return Value: -1 if error, else the number of DWT comparators.
 */
int DetectDwtResources()
{
    DWORD val, n, nAddr;
    int   status;

    memset(&DwtResources, 0, sizeof(DwtResources));

    if (DWT_Addr == 0) {
        DwtResources.NumComps        = 0;
        DwtResources.NumCycleComps   = 0;
        DwtResources.NumLink2ndComps = 0;
        DwtResources.NumLimitComps   = 0;
        DwtResources.NumLinkComps    = 0;
        DwtResources.NumValueComps   = 0;
        return 0;
    }

#if DBGCM_V8M
    status = ReadD32(DWT_CTRL, &val, BLOCK_SECTYPE_ANY);
    if (status) {
        OutErrorMessage(status);
        return (-1);
    }
#else  // DBGCM_V8M
    status = ReadD32(DWT_CTRL, &val);
    if (status) {
        OutErrorMessage(status);
        return (-1);
    }
#endif // DBGCM_V8M

    DwtResources.NumComps = (unsigned short)((val & DWT_NUMCOMP) >> DWT_NUMCOMP_P);


#if 0
  if ((val & DWT_NOEXTTRIG) == 0) {
    // CMPMATCH[n] is supported
    OutputDebugString("CMPMATCH[n] is supported\n");
  }
#endif

    if (IsV8M()) {
        for (n = 0; n < DwtResources.NumComps; n++) {
#if DBGCM_V8M
            nAddr  = DWT_FUNC + (n * 4 * 4);
            status = ReadD32(nAddr, &val, BLOCK_SECTYPE_ANY);
            if (status) {
                OutErrorMessage(status);
                continue;
            }
#else  // DBGCM_V8M
            nAddr  = DWT_FUNC + (n * 4 * 4);
            status = ReadD32(nAddr, &val);
            if (status) {
                OutErrorMessage(status);
                continue;
            }
#endif // DBGCM_V8M

            if ((val & DWTv8_ID_LIMIT) && n > 0) { // Workaround for incorrectly reported linking support in comparator 0
                DwtResources.NumLimitComps++;
            }
            //if (val & DWTv8_ID_DADDR) {           // All comparators are DAddr
            //}
            if ((val & DWTv8_ID_DVAL) && n > 0) { // Workaround for incorrectly reported linking support in comparator 0
                DwtResources.NumValueComps++;
            }
            if ((val & (DWTv8_ID_LIMIT | DWTv8_ID_DVAL)) && n > 0) { // Workaround for incorrectly reported linking support in comparator 0
                DwtResources.NumLinkComps++;
            }
            //if (val & DWTv8_ID_IADDR) {           // Ignore IAddr comparators
            //}
            if (val & DWTv8_ID_CYCCNT) {
                DwtResources.NumCycleComps++;
            }
        }
    } else {
#if DBGCM_V8M
        if (DwtResources.NumComps > 0) {
            // Test the first comparator for maximum bit mask
            status = WriteD32(DWT_MASK, DWT_MASK_MAX, BLOCK_SECTYPE_ANY);
            if (status) {
                OutErrorMessage(status);
                DwtResources.MaxMaskBits = 0x0F;
            } else {
                status = ReadD32(DWT_MASK, &val, BLOCK_SECTYPE_ANY);
                if (status) {
                    OutErrorMessage(status);
                    DwtResources.MaxMaskBits = 0x0F;
                } else {
                    DwtResources.MaxMaskBits = (val & DWT_MASK_MAX);
                }
                status = WriteD32(DWT_MASK, 0, BLOCK_SECTYPE_ANY); // clear the mask
                if (status)
                    OutErrorMessage(status);
            }
        }

        // Now check the rest
        for (n = 0; n < DwtResources.NumComps; n++) {
            nAddr = DWT_FUNC + (n * 4 * 4);
            // Test DATAVMATCH bit
            status = WriteD32(nAddr, DWT_DATAVMATCH, BLOCK_SECTYPE_ANY);
            if (status) {
                OutErrorMessage(status);
            } else {
                // Read register for data value matching caps
                status = ReadD32(nAddr, &val, BLOCK_SECTYPE_ANY);
                if (status) {
                    OutErrorMessage(status);
                } else {
                    if (val & DWT_DATAVMATCH) {
                        DwtResources.NumValueComps++;
                    }
                    if (val & DWT_LNK1ENA) {
                        DwtResources.NumLink2ndComps++;
                    }
                }
            }

            // Test CYCMATCH bit
            status = WriteD32(nAddr, DWT_CYCMATCH, BLOCK_SECTYPE_ANY);
            if (status) {
                OutErrorMessage(status);
            } else {
                // Read register for cycle matching caps
                status = ReadD32(nAddr, &val, BLOCK_SECTYPE_ANY);
                if (status) {
                    OutErrorMessage(status);
                } else {
                    if (val & DWT_CYCMATCH) {
                        DwtResources.NumCycleComps++;
                    }
                }
            }

            // Clear function register
            status = WriteD32(nAddr, DWT_DISABLED, BLOCK_SECTYPE_ANY);
            if (status) {
                OutErrorMessage(status);
            }
        }
#else  // DBGCM_V8M
        if (DwtResources.NumComps > 0) {
            // Test the first comparator for maximum bit mask
            status = WriteD32(DWT_MASK, DWT_MASK_MAX);
            if (status) {
                OutErrorMessage(status);
                DwtResources.MaxMaskBits = 0x0F;
            } else {
                status = ReadD32(DWT_MASK, &val);
                if (status) {
                    OutErrorMessage(status);
                    DwtResources.MaxMaskBits = 0x0F;
                } else {
                    DwtResources.MaxMaskBits = (val & DWT_MASK_MAX);
                }
                status = WriteD32(DWT_MASK, 0); // clear the mask
                if (status)
                    OutErrorMessage(status);
            }
        }

        // Now check the rest
        for (n = 0; n < DwtResources.NumComps; n++) {
            nAddr = DWT_FUNC + (n * 4 * 4);
            // Test DATAVMATCH bit
            status = WriteD32(nAddr, DWT_DATAVMATCH);
            if (status) {
                OutErrorMessage(status);
            } else {
                // Read register for data value matching caps
                status = ReadD32(nAddr, &val);
                if (status) {
                    OutErrorMessage(status);
                } else {
                    if (val & DWT_DATAVMATCH) {
                        DwtResources.NumValueComps++;
                    }
                    if (val & DWT_LNK1ENA) {
                        DwtResources.NumLink2ndComps++;
                    }
                }
            }

            // Test CYCMATCH bit
            status = WriteD32(nAddr, DWT_CYCMATCH);
            if (status) {
                OutErrorMessage(status);
            } else {
                // Read register for cycle matching caps
                status = ReadD32(nAddr, &val);
                if (status) {
                    OutErrorMessage(status);
                } else {
                    if (val & DWT_CYCMATCH) {
                        DwtResources.NumCycleComps++;
                    }
                }
            }

            // Clear function register
            status = WriteD32(nAddr, DWT_DISABLED);
            if (status) {
                OutErrorMessage(status);
            }
        }
#endif // DBGCM_V8M
    }
    return DwtResources.NumComps;
}


static void DetectEtmv3Resources()
{
#if 1
    EtmResources.Capabilities = 0;
    EtmResources.NumDwtInputs = 0;
#else // No ETM Support in this driver yet (may come with ETB)
    DWORD val;
    int   status;

    if (ETM_Addr == 0) {
        EtmResources.Capabilities = 0;
        EtmResources.NumDwtInputs = 0;
        return;
    }

    EtmResources.Capabilities = ETM_CAP_IMPLEMENTED;

#if DBGCM_V8M
    // Is Start/Stop logic available?
    status = ReadD32(ETMv3_CFGCODE, &val, BLOCK_SECTYPE_ANY);
    if (status) {
        OutErrorMessage(status);
    } else {
        if (val & ETMv3_CFGCODE_SS) {
            EtmResources.Capabilities |= ETM_CAP_START_STOP;
        }
    }

    // Determine number if EmbeddedICE inputs to ETM (DWT.CMPMATCH[n])
    status = ReadD32(ETMv3_CFGCODEEXT, &val, BLOCK_SECTYPE_ANY);
    if (status) {
        OutErrorMessage(status);
    } else {
        if (val & ETMv3_CFGCODEEXT_SS_ICE) {
            EtmResources.Capabilities |= ETM_CAP_START_STOP_DWT;
            EtmResources.NumDwtInputs = (val & ETMv3_CFGCODEEXT_NUM_ICE_M) >> ETMv3_CFGCODEEXT_NUM_ICE_P;
        }
    }
#else  // DBGCM_V8M
    // Is Start/Stop logic available?
    status = ReadD32(ETMv3_CFGCODE, &val);
    if (status) {
        OutErrorMessage(status);
    } else {
        if (val & ETMv3_CFGCODE_SS) {
            EtmResources.Capabilities |= ETM_CAP_START_STOP;
        }
    }

    // Determine number if EmbeddedICE inputs to ETM (DWT.CMPMATCH[n])
    status = ReadD32(ETMv3_CFGCODEEXT, &val);
    if (status) {
        OutErrorMessage(status);
    } else {
        if (val & ETMv3_CFGCODEEXT_SS_ICE) {
            EtmResources.Capabilities |= ETM_CAP_START_STOP_DWT;
            EtmResources.NumDwtInputs = (val & ETMv3_CFGCODEEXT_NUM_ICE_M) >> ETMv3_CFGCODEEXT_NUM_ICE_P;
        }
    }
#endif // DBGCM_V8M

#endif
}


static void DetectEtmv4Resources()
{
    if (ETM_Addr == 0) {
        EtmResources.Capabilities = 0;
        EtmResources.NumDwtInputs = 0;
        return;
    }

    // ETM Tracepoints not supported yet for ETMv4
    EtmResources.Capabilities = 0;
    EtmResources.NumDwtInputs = 0;
}


/*
 *  InitBreakResources():
 *    - Initializes resource structures
 *    - Collects capabilities from target registers
 */
unsigned long InitBreakResources()
{
    memset(&SwBreakResources, 0, sizeof(SwBreakResources));
    memset(&EtmResources, 0, sizeof(EtmResources));

    SwBreakResources.Allowed       = true;  // Always allow
    SwBreakResources.SetSWBreakRun = false; // Do not allow

    MergingRunBreaks = false;
    // Potential SW breakpoint resources
    //    Nothing to do, initialized by above memset

    // ETM resources
    // if (ETM_Version == 3) {
    //   DetectEtmv3Resources();
    // } else if (ETM_Version == 4) {
    //   DetectEtmv4Resources();
    // }
    // Need to assume that all ETMs have Start/Stop logic and four EICE inputs, many devices have
    // broken Configuration Code Registers...
    // M0 and M1 don't have an ETM, M3 and M4 do and all their revisions implement the below features
    if (ETM_Addr) {
        if (ETM_Version == 3) { // For ETMv3 only at the moment
            EtmResources.Capabilities = ETM_CAP_IMPLEMENTED | ETM_CAP_START_STOP | ETM_CAP_START_STOP_DWT;
            EtmResources.NumDwtInputs = DwtResources.NumComps;
        }
    }

    // Used Resources
    ClearUsedBreakResources();

    // SW Breakpoint Configurations
    SWBCHead = SWBCGarbageHead = NULL;


#if 0
  DumpBreakResources();
#endif
    return RES_OK;
}


/*
 *  CheckBreakResources():
 *    - Checks if required resources are supported by target
 *    - Checks if enough resources are available to fulfill the request
 */
unsigned long CheckBreakResources(unsigned short nReqType, unsigned short nFlags)
{
    unsigned short singleUse        = 0;
    unsigned short singleUseAlloced = 0;

    // ETM Capabilities
    if (nFlags & (BRK_RES_FLAG_ETM_START | BRK_RES_FLAG_ETM_STOP | BRK_RES_FLAG_ETM_TRIG | BRK_RES_FLAG_ETM_ENA_EVT)) {
        if ((EtmResources.Capabilities & ETM_CAP_IMPLEMENTED) == 0) {
            return RES_ERR_NO_ETM;
        }
        if (EtmResources.NumDwtInputs <= 0) {
            return RES_ERR_NO_ETM_DWT_IN;
        }
    }

    if ((nFlags & (BRK_RES_FLAG_ETM_START | BRK_RES_FLAG_ETM_STOP))) {
        if ((EtmResources.Capabilities & ETM_CAP_START_STOP) == 0) {
            return RES_ERR_NO_ETM_STARTSTOP;
        }
    }

    if (nFlags & (BRK_RES_FLAG_ETM_TRIG)) {
        if (EtmResources.NumTrigInputsAlloced >= MAX_ETM_TRIG_IN) {
            return RES_ERR_USED_ETM_TRIG_IN;
        }
    }


    if (nReqType == BRK_RES_REQ_PC) {
        // Address breakpoint
        if ((nFlags & BRK_RES_FLAG_CAN_SWBREAK) == 0) {
            // Hardware breakpoint only
            if (BreakResources.NumBreaks <= 0) {
                return RES_ERR_NO_BREAK;
            } else if (BreakResources.NumBreaksAlloced >= BreakResources.NumBreaks) {
                return RES_ERR_USED_BREAK;
            }
        }
    } else if (nReqType >= BRK_RES_REQ_DADDR && nReqType <= BRK_RES_REQ_CYCLE) {
        if (DwtResources.NumComps <= 0) {
            return RES_ERR_NO_WATCH;
        }

        switch (nReqType) {
            case BRK_RES_REQ_CYCLE:
                if (DwtResources.NumCycleComps <= 0) {
                    return RES_ERR_NO_WATCHCYCLE;
                }
                break;
            case BRK_RES_REQ_DLINKED2:
                if (IsV8M()) {
                    // Cannot linke more than two comparators for one function on v8-M
                    return RES_ERR_NO_WATCHLINK2;
                } else {
                    if (DwtResources.NumLink2ndComps <= 0) {
                        return RES_ERR_NO_WATCHLINK2;
                    }
                }
            case BRK_RES_REQ_DLINKED1:
                if (IsV8M()) {
                    if (DwtResources.NumValueComps <= 0 || DwtResources.NumLinkComps <= 0) {
                        return RES_ERR_NO_WATCHLINK1;
                    }
                } else {
                    if (DwtResources.NumValueComps <= 0) {
                        return RES_ERR_NO_WATCHVAL;
                    }
                }
                break;
            case BRK_RES_REQ_DVALUE:
                if (DwtResources.NumValueComps <= 0) {
                    return RES_ERR_NO_WATCHVAL;
                }
                break;

            case BRK_RES_REQ_DRANGE:
                if (IsV8M()) {
                    if (DwtResources.NumLimitComps <= 0 || DwtResources.NumLinkComps <= 0) {
                        return RES_ERR_NO_WATCHLINK1;
                    }
                } else {
                    return RES_ERR_NO_WATCHLINK1;
                }
                break;
        }


        // Check DWT number in general
        switch (nReqType) {
            case BRK_RES_REQ_CYCLE:
            case BRK_RES_REQ_DADDR:
            case BRK_RES_REQ_DVALUE:
                // one dwt comp
                if (DwtResources.NumComps - DwtResources.NumCompsAlloced < 1) {
                    return RES_ERR_USED_WATCH;
                }
                break;
            case BRK_RES_REQ_DLINKED1:
            case BRK_RES_REQ_DRANGE:
                if (DwtResources.NumComps - DwtResources.NumCompsAlloced < 2) {
                    return RES_ERR_USED_WATCH;
                }
                break;
            case BRK_RES_REQ_DLINKED2:
                if (IsV8M()) {
                    // Not supported on v8-M
                    return RES_ERR_USED_WATCH;
                } else {
                    if (DwtResources.NumComps - DwtResources.NumCompsAlloced < 3) {
                        return RES_ERR_USED_WATCH;
                    }
                }
                break;
        }

        // Check number of specialized resources
        switch (nReqType) {
            case BRK_RES_REQ_CYCLE:
                if (DwtResources.NumCycleCompsAlloced >= DwtResources.NumCycleComps) {
                    return RES_ERR_USED_WATCHCYCLE;
                }
                break;
            case BRK_RES_REQ_DLINKED2:
                if (IsV8M()) {
                    // Cannot linke more than two comparators for one function on v8-M
                    return RES_ERR_NO_WATCHLINK2;
                } else {
                    if (DwtResources.NumLink2ndCompsAlloced >= DwtResources.NumLink2ndComps) {
                        return RES_ERR_USED_WATCHLINK2;
                    }
                    if (DwtResources.NumValueCompsAlloced >= DwtResources.NumValueComps) {
                        return RES_ERR_USED_WATCHVAL;
                    }
                }
                break;
            case BRK_RES_REQ_DLINKED1:
                if (IsV8M()) {
                    if (DwtResources.NumValueCompsAlloced >= DwtResources.NumValueComps) {
                        return RES_ERR_USED_WATCHLINK1;
                    }
                    if (DwtResources.NumLinkCompsAlloced >= DwtResources.NumLinkComps) {
                        return RES_ERR_USED_WATCHLINK1;
                    }
                    // Odd comparators are recommended as linkable => Overall number of comparators >= 2*number of linkable comparators
                    singleUse        = DwtResources.NumComps - 2 * DwtResources.NumLinkComps;
                    singleUseAlloced = DwtResources.NumCompsAlloced - 2 * DwtResources.NumLinkCompsAlloced;
                    if (singleUseAlloced > singleUse) { // More comparators allocated for single use than solely available for single use
                        if (singleUseAlloced - singleUse > 2 * (DwtResources.NumLinkComps - DwtResources.NumLinkCompsAlloced)) {
                            // Cannot fit another pair into current setup
                            return RES_ERR_USED_WATCH;
                        }
                    }
                } else {
                    if (DwtResources.NumValueCompsAlloced >= DwtResources.NumValueComps) {
                        return RES_ERR_USED_WATCHVAL;
                    }
                }
                break;
            case BRK_RES_REQ_DVALUE:
                if (DwtResources.NumValueCompsAlloced >= DwtResources.NumValueComps) {
                    return RES_ERR_USED_WATCHVAL;
                }
                if (IsV8M()) {
                    if (DwtResources.NumLinkCompsAlloced >= DwtResources.NumLinkComps) {
                        return RES_ERR_USED_WATCHVAL;
                    }
                }
                break;
            case BRK_RES_REQ_DRANGE:
                if (IsV8M()) {
                    if (DwtResources.NumLimitCompsAlloced >= DwtResources.NumLimitComps) {
                        return RES_ERR_USED_WATCHLINK1;
                    }
                    if (DwtResources.NumLinkCompsAlloced >= DwtResources.NumLinkComps) {
                        return RES_ERR_USED_WATCHLINK1;
                    }
                    // Odd comparators are recommended as linkable => Overall number of comparators >= 2*number of linkable comparators
                    singleUse        = DwtResources.NumComps - 2 * DwtResources.NumLinkComps;
                    singleUseAlloced = DwtResources.NumCompsAlloced - 2 * DwtResources.NumLinkCompsAlloced;
                    if (singleUseAlloced > singleUse) { // More comparators allocated for single use than solely available for single use
                        if (singleUseAlloced - singleUse > 2 * (DwtResources.NumLinkComps - DwtResources.NumLinkCompsAlloced)) {
                            // Cannot fit another pair into current setup
                            return RES_ERR_USED_WATCH;
                        }
                    }
                } else {
                    return RES_ERR_USED_WATCHLINK1;
                }
                break;
        }

    } else {
        return RES_ERR_UNKNOWN;
    }

    return RES_OK;
}


/*
 *  CheckWatchAddress():
 *    - Checks a given watch range for alignment and size
 *    - Decides whether the given range is usable with hardware
 */
unsigned long CheckWatchAddress(unsigned long nAddr, unsigned long nMany)
{
    unsigned long nMaskBits;

    if (IsV8M()) {
        // Not required for v8-M
        return RES_OK;
    }

    for (nMaskBits = 0; nMaskBits < DwtResources.MaxMaskBits; nMaskBits++) {
        if ((1UL << nMaskBits) >= nMany) {
            break;
        }
    }

    if (nMaskBits >= DwtResources.MaxMaskBits) {
        return RES_ERR_RNG_SIZE;
    }

    if ((1UL << nMaskBits) != nMany) {
        return RES_ERR_RNG_UNALIGNED;
    }

    if ((nAddr & ((1UL << nMaskBits) - 1)) != 0) {
        return RES_ERR_WADDR_UNALIGNED;
    }

    return RES_OK;
}


/*
 *  AdjustWatchAddress():
 *    - Adjusts the requested address range to the closest valid address range.
 *    - Parameters:
 *      - pB: Breakpoint for which to modify the address range to achieve alignment.
 */
unsigned long AdjustWatchAddress(AG_BP *pB)
{
    unsigned long nMaskBits;
    unsigned long nMask;
    unsigned long nStartAddr = pB->Adr;
    unsigned long nAccSize   = pB->tsize;
    unsigned long nBytes     = nAccSize * pB->many;
    unsigned long nEndAddr   = nStartAddr + nBytes - 1;

    if (IsV8M()) {
        // Not required for v8-M
        return RES_OK;
    }

    // Find common address offset and mask bits
    for (nMaskBits = 0, nMask = 0xFFFFFFFF; nMaskBits < DwtResources.MaxMaskBits; nMaskBits++) {
        if ((nStartAddr & nMask) == (nEndAddr & nMask)) {
            break;
        }
        nMask &= ~(1UL << nMaskBits);
    }

    if (nMaskBits >= DwtResources.MaxMaskBits) {
        // Range is too large, requested range can never be watched
        return RES_ERR_RNG_SIZE;
    }

    // Get new address range
    nStartAddr &= nMask;
    nEndAddr = (nEndAddr & nMask) | (~nMask);
    nBytes   = nEndAddr - nStartAddr + 1;

    if (nAccSize > 4) {
        nAccSize = 4;
    }
#if _DEBUG
    if (nBytes % nAccSize) {
        TRACE("ERROR IN ALIGNMENT!!\n");
    }
#endif

    // Store the original values and indicate that the watch has been modified
    pB->TP_INFO |= TP_INFO_WMOD;
    pB->TP_ORG_ADDR = pB->Adr;
    pB->TP_ORG_LEN  = pB->many * pB->tsize;

    pB->Adr   = nStartAddr;
    pB->tsize = nAccSize;
    pB->many  = nBytes / nAccSize; // at this point nBytes should be a multiple of nAccSz
    return RES_OK;
}


/*
 *  AllocBreakResources(nReqType, nFlags):
 *   - Calls CheckBreakResources()
 *   - Allocates resources (increments used-counters)
 */
unsigned long AllocBreakResources(unsigned short nReqType, unsigned short nFlags)
{
    unsigned long nCheckRes = 0;

    nCheckRes = CheckBreakResources(nReqType, nFlags);
    if (nCheckRes != 0) {
        return nCheckRes;
    }

    switch (nReqType) {
        case BRK_RES_REQ_PC:
            if (nFlags & BRK_RES_FLAG_CAN_SWBREAK) {
                SwBreakResources.NumSWBreaks++;
            } else if (BreakResources.NumBreaksAlloced < BreakResources.NumBreaks) {
                BreakResources.NumBreaksAlloced++;
            }
            break;
        case BRK_RES_REQ_CYCLE:
            DwtResources.NumCompsAlloced++;
            DwtResources.NumCycleCompsAlloced++;
            break;
        case BRK_RES_REQ_DLINKED2:
            if (IsV8M()) {
                // Cannot linke more than two comparators for one function on v8-M
                return RES_ERR_NO_WATCHLINK2;
            } else {
                DwtResources.NumCompsAlloced += 3;
                DwtResources.NumLink2ndCompsAlloced++;
                DwtResources.NumValueCompsAlloced++;
            }
            break;
        case BRK_RES_REQ_DLINKED1:
            DwtResources.NumCompsAlloced += 2;
            DwtResources.NumValueCompsAlloced++;
            if (IsV8M()) {
                DwtResources.NumLinkCompsAlloced++; // Value comparator is always linkable
            }
            break;
        case BRK_RES_REQ_DVALUE:
            if (IsV8M()) {
                DwtResources.NumCompsAlloced++;
                DwtResources.NumValueCompsAlloced++;
                DwtResources.NumLinkCompsAlloced++; // Not used as such but gone anyway
            } else {
                DwtResources.NumValueCompsAlloced++;
                DwtResources.NumCompsAlloced++;
            }
            break;
        case BRK_RES_REQ_DADDR:
            DwtResources.NumCompsAlloced++;
            break;
        case BRK_RES_REQ_DRANGE:
            if (IsV8M()) {
                DwtResources.NumCompsAlloced += 2;
                DwtResources.NumLimitCompsAlloced++;
                DwtResources.NumLinkCompsAlloced++;
            } else {
                return RES_ERR_NO_WATCHLINK1;
            }
            break;
    }

    if (nFlags & BRK_RES_FLAG_ETM_TRIG) {
        EtmResources.NumTrigInputsAlloced++;
    }

    return RES_OK;
}


/*
 *  AllocBreakResources(nAlloc, pB):
 *   - Translates a breakpoint request into request type and flags
 *   - Checks/sets AG_BP.bAlloced for later reference
 *   - Tests address of an ABREAK for possibility to set an SW break
 *   - Calls lower layer alloc/free function
 *   - nAlloc: 0 - free, 1 - alloc, 2 - address adjustment
 */
unsigned long AllocBreakResources(DWORD nAlloc, AG_BP *pB)
{
    unsigned short nReqType    = 0;
    unsigned short nFlags      = 0;
    unsigned long  nStatus     = RES_OK;
    BOOL           bCanSWBreak = FALSE;


    // Prepare parameters for upcoming call
    switch (pB->type) {
        case AG_ABREAK:
            nReqType = BRK_RES_REQ_PC;
            break;
        case AG_CBREAK:
            // NOT SUPPORTED
#if _DEBUG
            TRACE("Conditional Breakpoints not supported.\n");
#endif
            return RES_ERR_NOTSUPPORTED;
        case AG_WBREAK:
            if (IsV8M()) {
                if ((pB->acc > 0) && (pB->TP_INFO & TP_INFO_WVAL)) { // same as BP_INFO and BP_INFO_WVAL
                    nReqType = BRK_RES_REQ_DLINKED1;
                } else if (pB->many * pB->tsize > 4 || pB->many * pB->tsize == 3) {
                    nReqType = BRK_RES_REQ_DRANGE;
                } else {
                    if (pB->Adr & ((pB->many * pB->tsize) - 1)) {
                        // Unaligned address, try range for v8-M
                        nReqType = BRK_RES_REQ_DRANGE;
                    } else {
                        nReqType = BRK_RES_REQ_DADDR;
                    }
                }
            } else {
                if ((pB->acc > 0) && (pB->TP_INFO & TP_INFO_WVAL)) { // same as BP_INFO and BP_INFO_WVAL
                    nReqType = BRK_RES_REQ_DLINKED1;
                } else {
                    nReqType = BRK_RES_REQ_DADDR;
                }
            }
            break;
        case AG_TBREAK:
            switch (pB->TP_TYPE) {
                case 1:
                    nFlags |= BRK_RES_FLAG_ETM_START;
                    break;
                case 2:
                    nFlags |= BRK_RES_FLAG_ETM_STOP;
                    break;
                case 3:
                    nFlags |= BRK_RES_FLAG_ETM_TRIG;
                    if (pB->TP_EXT_PTR) {
                        nStatus = AllocBreakResources(nAlloc, (AG_BP *)pB->TP_EXT_PTR);
                        if (nStatus != RES_OK)
                            return nStatus;
                    }
                    break;
                    // Nothing to do for TraceDataPoint and TraceAccessPoint
            }

            if (IsV8M()) {
                if ((pB->acc > 0) && (pB->TP_INFO & TP_INFO_WVAL)) {
                    nReqType = BRK_RES_REQ_DLINKED1;
                } else if (pB->many * pB->tsize > 4 || pB->many * pB->tsize == 3) {
                    nReqType = BRK_RES_REQ_DRANGE;
                    return ((pB->many * pB->tsize == 3) ? RES_ERR_RNG_UNALIGNED : RES_ERR_RNG_SIZE); // TODO: Temporary until supporting range tracepoints
                } else {
                    if (pB->Adr & ((pB->many * pB->tsize) - 1)) {
                        // Unaligned address, try range for v8-M
                        nReqType = BRK_RES_REQ_DRANGE;
                        return RES_ERR_RNG_UNALIGNED; // TODO: Temporary until supporting range tracepoints
                    } else {
                        nReqType = BRK_RES_REQ_DADDR;
                    }
                }
            } else {
                if ((pB->acc > 0) && (pB->TP_INFO & TP_INFO_WVAL)) {
                    nReqType = BRK_RES_REQ_DLINKED1;
                } else {
                    nReqType = BRK_RES_REQ_DADDR;
                }
            }

            break;
    }


    // Check if breakpoint resource is already allocated
    if (nAlloc == 1 && (pB->bAlloced || IsRunBreak(pB->Adr))) {
        // skip
        return RES_OK;
    }

    // Check if breakpoint resource is already freed
    if (nAlloc == 0 && !pB->bAlloced && !IsRunBreak(pB->Adr)) {
        // skip
        return RES_OK;
    }

    if (!IsV8M()) {
        // Check address range for WBREAKs and TBREAKs
        if (nAlloc > 0 && (pB->type == WBREAK || pB->type == TBREAK)) {
            nStatus = CheckWatchAddress(pB->Adr, pB->many * pB->tsize);
            if (nStatus != RES_OK) {
                if (pB->type == TBREAK) { // only do address adjustment for TBREAKs
                    switch (nStatus) {
                        case RES_ERR_WADDR_UNALIGNED:
                        case RES_ERR_RNG_UNALIGNED:
                        case RES_ERR_RNG_SIZE:
                            // Try to correct the range to be properly aligned
                            nStatus = AdjustWatchAddress(pB);
                            break;
                        default:
                            // do nothing
                            break;
                    }
                }

                if (nStatus != RES_OK) {
                    // correcting the address didn't help either, abort
                    if (nAlloc == 1 && pB->type == AG_TBREAK && pB->TP_TYPE == TP_TYPE_HALT && pB->TP_EXT_PTR) {
                        // revert the already done allocation
                        AllocBreakResources(0, (AG_BP *)pB->TP_EXT_PTR);
                    }
                    return nStatus;
                }

                // correcting the address did the trick for a TBREAK, continue
            }
        }
    }

    if (nAlloc == 2) {
        // only do the address range adjustment, leave now
        return RES_OK;
    }

    // Check for ABREAKs if a software break is possible
    if (pB->type == ABREAK) {
        // Breakpoint Handling if running
        if (iRun && !MergingRunBreaks && !SwBreakResources.SetSWBreakRun) {
            if (nAlloc == 1) { // Allocate
                nStatus = AddRunBreak(pB->Adr);
                return (nStatus);
            } else if (nAlloc == 0) { // Free
                nStatus = FreeRunBreak(pB->Adr);
                if (nStatus != RES_ERR_NO_ALLOC)
                    return (nStatus);
                nStatus = RES_OK; // Free resource allocated while target was stopped
            }
        }

        if (CanSwBreak(pB)) {
            nFlags |= BRK_RES_FLAG_CAN_SWBREAK;
        } else {
            nStatus = CheckHWBreakAddress(pB->Adr);
            if (nStatus != RES_OK)
                return (nStatus);
        }
    }

    if (nAlloc == 1) {
        if (pB->type != ABREAK || !BreakOnWord(pB, (nFlags & BRK_RES_FLAG_CAN_SWBREAK))) { // no ABREAK or break comp already in use
            nStatus = AllocBreakResources(nReqType, nFlags);
        }
        if (nStatus == RES_OK)
            pB->bAlloced = 1;
    } else if (nAlloc == 0) {
        if (pB->type != ABREAK || !OtherBreakOnWord(pB, (nFlags & BRK_RES_FLAG_CAN_SWBREAK))) { // no ABREAK or break comp no longer in use
            nStatus = FreeBreakResources(nReqType, nFlags);
        }
        if (nStatus == RES_OK)
            pB->bAlloced = 0;
    }

    // clean up if allocation failed
    if (nStatus != RES_OK) {
        if (pB->type == AG_TBREAK && pB->TP_TYPE == TP_TYPE_HALT && pB->TP_EXT_PTR) {
            // revert the already done allocation
            if (nAlloc == 1) {
                AllocBreakResources(0, (AG_BP *)pB->TP_EXT_PTR);
            }
        }
    }

#if 0
  DumpBreakResources();
#endif

    return nStatus;
}


/*
 *  FreeBreakResources(): Frees the allocated resources of a single request if previously set.
 */
unsigned long FreeBreakResources(unsigned short nReqType, unsigned short nFlags)
{
    unsigned long nErrCode = RES_OK;

    if (nFlags & BRK_RES_FLAG_ETM_TRIG) {
        if (EtmResources.NumTrigInputsAlloced <= 0) {
            nErrCode = RES_ERR_NO_ALLOC;
        } else {
            EtmResources.NumTrigInputsAlloced--;
        }
    }


    switch (nReqType) {
        case BRK_RES_REQ_PC:
            if (nFlags & BRK_RES_FLAG_CAN_SWBREAK) {
                if (SwBreakResources.NumSWBreaks <= 0) {
                    nErrCode = RES_ERR_NO_ALLOC;
                    break;
                }

                SwBreakResources.NumSWBreaks--;
                break;
            }

            if (BreakResources.NumBreaksAlloced <= 0) {
                nErrCode = RES_ERR_NO_ALLOC;
            } else {
                BreakResources.NumBreaksAlloced--;
            }
            break;
        case BRK_RES_REQ_CYCLE:
            if (DwtResources.NumCompsAlloced <= 0) {
                nErrCode = RES_ERR_NO_ALLOC;
            }
            if (DwtResources.NumCycleCompsAlloced <= 0) {
                nErrCode = RES_ERR_NO_ALLOC;
            }

            if (nErrCode == RES_OK) {
                DwtResources.NumCompsAlloced--;
                DwtResources.NumCycleCompsAlloced--;
            }
            break;
        case BRK_RES_REQ_DLINKED2:
            if (IsV8M()) {
                return RES_ERR_NO_ALLOC; // Not supported by v8-M
            } else {
                if (DwtResources.NumCompsAlloced < 3) {
                    nErrCode = RES_ERR_NO_ALLOC;
                }
                if (DwtResources.NumLink2ndCompsAlloced <= 0) {
                    nErrCode = RES_ERR_NO_ALLOC;
                }
                if (DwtResources.NumValueCompsAlloced <= 0) {
                    nErrCode = RES_ERR_NO_ALLOC;
                }

                if (nErrCode == RES_OK) {
                    DwtResources.NumCompsAlloced -= 3;
                    DwtResources.NumLink2ndCompsAlloced--;
                    DwtResources.NumValueCompsAlloced--;
                }
            }
            break;
        case BRK_RES_REQ_DLINKED1:
            if (IsV8M()) {
                if (DwtResources.NumCompsAlloced < 2) {
                    nErrCode = RES_ERR_NO_ALLOC;
                }
                if (DwtResources.NumValueCompsAlloced < 1) {
                    nErrCode = RES_ERR_NO_ALLOC;
                }
                if (DwtResources.NumLinkCompsAlloced < 1) {
                    nErrCode = RES_ERR_NO_ALLOC;
                }

                if (nErrCode == RES_OK) {
                    DwtResources.NumCompsAlloced -= 2;
                    DwtResources.NumValueCompsAlloced--;
                    DwtResources.NumLinkCompsAlloced--;
                }
            } else {
                if (DwtResources.NumCompsAlloced < 2) {
                    nErrCode = RES_ERR_NO_ALLOC;
                }
                if (DwtResources.NumValueCompsAlloced <= 0) {
                    nErrCode = RES_ERR_NO_ALLOC;
                }

                if (nErrCode == RES_OK) {
                    DwtResources.NumCompsAlloced -= 2;
                    DwtResources.NumValueCompsAlloced--;
                }
            }
            break;
        case BRK_RES_REQ_DVALUE:
            if (IsV8M()) {
                if (DwtResources.NumCompsAlloced <= 0) {
                    nErrCode = RES_ERR_NO_ALLOC;
                }
                if (DwtResources.NumValueCompsAlloced < 1) {
                    nErrCode = RES_ERR_NO_ALLOC;
                }
                if (DwtResources.NumLinkCompsAlloced < 1) {
                    nErrCode = RES_ERR_NO_ALLOC;
                }
                if (nErrCode == RES_OK) {
                    DwtResources.NumCompsAlloced--;
                    DwtResources.NumValueCompsAlloced--;
                    DwtResources.NumLinkCompsAlloced--;
                }
            } else {
                if (DwtResources.NumCompsAlloced <= 0) {
                    nErrCode = RES_ERR_NO_ALLOC;
                }
                if (DwtResources.NumValueCompsAlloced <= 0) {
                    nErrCode = RES_ERR_NO_ALLOC;
                }

                if (nErrCode == RES_OK) {
                    DwtResources.NumCompsAlloced--;
                    DwtResources.NumValueCompsAlloced--;
                }
            }
            break;
        case BRK_RES_REQ_DADDR:
            if (IsV8M()) {
                if (DwtResources.NumCompsAlloced < 1) {
                    nErrCode = RES_ERR_NO_ALLOC;
                }
            } else {
                if (DwtResources.NumCompsAlloced <= 0) {
                    nErrCode = RES_ERR_NO_ALLOC;
                }
            }
            if (nErrCode == RES_OK) {
                DwtResources.NumCompsAlloced--;
            }
            break;
        case BRK_RES_REQ_DRANGE:
            if (IsV8M()) {
                if (DwtResources.NumCompsAlloced < 2) {
                    nErrCode = RES_ERR_NO_ALLOC;
                }
                if (DwtResources.NumLimitCompsAlloced < 1) {
                    nErrCode = RES_ERR_NO_ALLOC;
                }
                if (DwtResources.NumLinkCompsAlloced < 1) {
                    nErrCode = RES_ERR_NO_ALLOC;
                }
                if (nErrCode == RES_OK) {
                    DwtResources.NumCompsAlloced -= 2;
                    DwtResources.NumLimitCompsAlloced--;
                    DwtResources.NumLinkCompsAlloced--;
                }
            } else {
                return RES_ERR_NO_ALLOC;
            }
            break;
    }

    return nErrCode;
}


/*
 *  FreeAllResources(): Frees all allocated resources
 */
unsigned long FreeAllResources()
{
    BreakResources.NumBreaksAlloced   = 0;
    BreakResources.NumLiteralsAlloced = 0;

    DwtResources.NumCompsAlloced        = 0;
    DwtResources.NumCycleCompsAlloced   = 0;
    DwtResources.NumValueCompsAlloced   = 0;
    DwtResources.NumLink2ndCompsAlloced = 0;
    DwtResources.NumLimitCompsAlloced   = 0;
    DwtResources.NumLinkCompsAlloced    = 0;
    DwtResources.NumCompsBreakAlloced   = 0;

    //ClearSWBreaks();  // Keep the info to avoid unnecessary access to target for same address
    SwBreakResources.NumSWBreaks = 0;

    EtmResources.NumTrigInputsAlloced = 0;
    return RES_OK;
}


/*
 *  CanValueWatch(): Returns TRUE if DWT has a value matcher, else FALSE
 */
BOOL CanValueWatch()
{
    return (DwtResources.NumValueComps > 0);
}


/*
* GetDWTValueComps(): Return number of available Value Comparators in DWT.
*/
unsigned long GetDWTValueComps()
{
    return (DwtResources.NumValueComps);
}


/*
* GetDWTLinkComps(): Return number of available linkable Comparators in DWT.
*/
unsigned long GetDWTLinkComps()
{
    return (DwtResources.NumLinkComps);
}


/*
* GetDWTLinkComps(): Return number of available address limit Comparators in DWT.
*/
unsigned long GetDWTLimitComps()
{
    return (DwtResources.NumLimitComps);
}


/*
* GetDWTCycleComps(): Return number of available Cycle Comparators in DWT.
*/
unsigned long GetDWTCycleComps()
{
    return (DwtResources.NumCycleComps);
}


/*
 * ClearUsedResources(): Clears Information about resources used during a target run.
 *                       Call before and/or after target run/step.
 */
__inline void ClearUsedBreakResources()
{
    memset(&UsedBreakResources, 0, sizeof(UsedBreakResources));
}


/*
 * IncUsedBreakResources(): Increment number of actually used breakpoint resources.
 */
__inline unsigned long IncUsedBreakResources()
{
    if (UsedBreakResources.NumBreaks >= BreakResources.NumBreaks) {
        return (RES_ERR_USED_BREAK);
    }
    UsedBreakResources.NumBreaks++;
    return (RES_OK);
}


/*
 * DecUsedBreakResources(): Decrement number of actually used breakpoint resources.
 */
__inline unsigned long DecUsedBreakResources()
{
    if (UsedBreakResources.NumBreaks <= 0) {
        return (RES_ERR_NO_ALLOC);
    }
    UsedBreakResources.NumBreaks--;
    return (RES_OK);
}

// SW Breakpoint Configurations

static int SwBreakConfigGet(U16 id, AG_SWBREAKCONF_ITEM **ppitem)
{
    AG_SWBREAKCONF_ITEM *it;

    for (it = SWBCHead; it != NULL; it = it->next) {
        if (it->nID == id) {
            break;
        }
    }
    *ppitem = it;
    return (0);
}

static int SwBreakConfigRenumerate()
{
    AG_SWBREAKCONF_ITEM *it;
    U16                  n = 0;

    for (it = SWBCHead; it != NULL; it = it->next) {
        it->nID = n++;
    }
}


// Invalidate all breaks if item == NULL (not used at the moment)
static void SwBreaksInvalidate(const AG_SWBREAKCONF_ITEM *item)
{
    SW_BREAK *swbrk = SwBreakResources.pSwBreaks;
    for (; swbrk != NULL; swbrk = swbrk->pNext) {
        if (item) {
            if (swbrk->nAddr >= item->nStartAddr && swbrk->nAddr <= item->nEndAddr) {
                swbrk->bValid = 0;
            }
        } else {
            swbrk->bValid = 0;
        }
    }
}


/*
 * SwBreakConfGetHead(): Get head of SW Breakpoing Configuration List
 */
int SwBreakConfGetHead(AG_SWBREAKCONF_ITEM **pphead)
{
    *pphead = SWBCHead;
    return (AG_SWBC_ERR_OK);
}


int static SwBreakConfRangeDirty(AG_SWBREAKCONF_ITEM *item)
{
    AG_BP *pB;

    if (pBhead == NULL) { // Crash during flash download with SBC command in INI script
        return (0);
    }

    for (pB = *pBhead; pB != NULL; pB = pB->next) {
        if (pB->type == ABREAK && pB->bAlloced && !pB->bKilled) {
            if (pB->Adr >= item->nStartAddr && pB->Adr <= item->nEndAddr) {
                // Breakpoint set in range
                return (1);
            }
        }
    }
    return (0);
}


static int SwBreakConfCheckParams(const AG_SWBREAKCONF_ITEM *item)
{
    AP_CONTEXT *apCtx;
    int         status;
    // General parameter checks done in SARMCM3.DLL (or comparable)

    status = AP_CpuCtx(&apCtx); // 08.11.2018: Always use CPU's AP context
    if (status)
        return (AG_SWBC_ERR_INTERNAL);

    switch (item->nAccSz) { // Further target specific checks
        case 0:
            break;
        case 1:
            // if ((AP_AccSizes & AP_ACCSZ_BYTE) == 0) {
            if ((apCtx->AccSizes & AP_ACCSZ_BYTE) == 0) {
                return (AG_SWBC_ERR_ACCSZ);
            }
            break;
        case 2:
            // if ((AP_AccSizes & AP_ACCSZ_HWORD) == 0) {
            if ((apCtx->AccSizes & AP_ACCSZ_HWORD) == 0) {
                return (AG_SWBC_ERR_ACCSZ);
            }
            break;
        case 4:
            // if ((AP_AccSizes & AP_ACCSZ_WORD) == 0) {
            if ((apCtx->AccSizes & AP_ACCSZ_WORD) == 0) {
                return (AG_SWBC_ERR_ACCSZ);
            }
            break;
        default:
            return (AG_SWBC_ERR_ACCSZ);
    }

    if (item->nAccAlign > 0) {
        if (item->nAccAlign > 1024) { // Potentially debugger/target specific in future...
            return (AG_SWBC_ERR_ALIGNMAX);
        }
    }
    return (AG_SWBC_ERR_OK);
}



/*
 * SwBreakConfAdd(): Add new SW Breakpoint Item to list
 */
int SwBreakConfAdd(AG_SWBREAKCONF_ITEM *item)
{
    AG_SWBREAKCONF_ITEM *it, *last;
    U16                  n = 0;
    int                  nE;

    if (item == NULL) {
        return (AG_SWBC_ERR_INTERNAL);
    }

    // Check item parameters
    nE = SwBreakConfCheckParams(item);
    if (nE != AG_SWBC_ERR_OK)
        return (nE);

    for (it = SWBCHead, last = NULL; it != NULL; it = it->next, n++) {
        // Check for existing overlaps
        if ((item->nStartAddr >= it->nStartAddr && item->nStartAddr <= it->nEndAddr)
            || (item->nEndAddr >= it->nStartAddr && item->nEndAddr <= it->nEndAddr)) {
            return (AG_SWBC_ERR_ADDR_OVL);
        }
        last = it;
    }

    // Check if Breakpoints set in range
    if (item->bEnableSWBreaks == 0) {
        if (SwBreakConfRangeDirty(item)) {
            // cannot disable SW breaks while breakpoints set in range
            return (AG_SWBC_ERR_SET_DIRTY);
        }
    }

    // Append
    if (SWBCGarbageHead == NULL) {
        it = (AG_SWBREAKCONF_ITEM *)pio->GetMem(sizeof(AG_SWBREAKCONF_ITEM), ENV_DBM);
    } else {
        it              = SWBCGarbageHead;
        SWBCGarbageHead = SWBCGarbageHead->next; // Must be nulled by remove function!!
    }
    if (it == NULL) {
        return (AG_SWBC_ERR_INTERNAL);
    }

    memcpy(it, item, sizeof(AG_SWBREAKCONF_ITEM));
    it->nID  = n;
    it->next = NULL;
    if (last == NULL) { // Equal to (SWBCHead == NULL)
        SWBCHead = it;
    } else {
        last->next = it;
    }

    // Mark already tested and affected SW Breaks as invalid
    SwBreaksInvalidate(item);

    return (AG_SWBC_ERR_OK);
}


/*
 * SwBreakConfRem(): Remove SW Breakpoint Item from list.
 * Only uses 'nId' field of 'item', removes all items if 'item' is NULL.
 */
int SwBreakConfRem(AG_SWBREAKCONF_ITEM *item)
{
    AG_SWBREAKCONF_ITEM *it, *last, *git;
    int                  n     = 0;
    BYTE                 found = 0;

    if (item == NULL) {
        // First pass, check if any breaks set on SW break conf range
        for (it = SWBCHead; it != NULL; it = it->next) {
            if (it->bEnableSWBreaks == 0 && SwBreakConfRangeDirty(it)) {
                // cannot remove SW Break Conf (enable SW breaks) while breakpoints set in range
                return (AG_SWBC_ERR_REM_DIRTY);
            }
        }

        // Second pass, mark already tested and affected SW Breaks as invalid
        for (it = SWBCHead; it != NULL; it = it->next) {
            SwBreaksInvalidate(it);
        }

        if (SWBCGarbageHead == NULL) {
            SWBCGarbageHead = SWBCHead;
        } else {
            // Get last garbage item
            for (git = SWBCGarbageHead; (git && git->next); git = git->next)
                ;
            git->next = SWBCHead;
        }
        SWBCHead = NULL;

        return (AG_SWBC_ERR_OK);
    }

    // Get last garbage item
    for (git = SWBCGarbageHead; (git && git->next); git = git->next)
        ;

    for (it = SWBCHead, last = NULL; it != NULL; it = it->next) {
        if (found) {
            // Renumerate followig SBCs
            it->nID = n++;
        } else if (it->nID == item->nID) {
            // found the item to remove
            found = 1;

            // Check if any breaks set on SW break conf range
            if (it->bEnableSWBreaks == 0 && SwBreakConfRangeDirty(it)) {
                // cannot remove SW Break Conf (enable SW breaks) while breakpoints set in range
                return (AG_SWBC_ERR_REM_DIRTY);
            }

            // Mark already tested and affected SW Breaks as invalid
            SwBreaksInvalidate(it);

            // Update garbage pointers
            if (git == NULL) {
                git = SWBCGarbageHead = it;
            } else {
                git->next = it;
                git       = git->next;
            }

            // Unlink item
            it = it->next;
            if (last == NULL) {
                SWBCHead = it;
            } else {
                last->next = it;
            }
            if (git != NULL)
                git->next = NULL;

            // Exit loop if last item removed (otherwise for-expression can crash)
            if (it == NULL)
                break;

            // Start renumeration of following items
            it->nID = n++;
        } else {
            // not found yet
            n++;
            last = it;
        }
    }

    return (found ? AG_SWBC_ERR_OK : AG_SWBC_ERR_REM_ID);
}


const AG_SWBREAKCONF_ITEM *SwBreakGetConf(DWORD addr)
{
    AG_SWBREAKCONF_ITEM *conf;
    for (conf = SWBCHead; conf != NULL; conf = conf->next) {
        if (addr >= conf->nStartAddr && addr <= conf->nEndAddr) {
            break;
        }
    }
    return (conf);
}


/*
 * Invalidate Caches for Breakpoint Resources
 */
void InvalidateBreakResources()
{
    SwBreaksInvalidate(NULL);
}


void CrtInitBreakResources()
{
    memset(&BreakResources, 0, sizeof(BreakResources));
    memset(&DwtResources, 0, sizeof(DwtResources));
    memset(&SwBreakResources, 0, sizeof(SwBreakResources));
    memset(&EtmResources, 0, sizeof(EtmResources));
    memset(&RunBreakResources, 0, sizeof(RunBreakResources));

    memset(&UsedBreakResources, 0, sizeof(UsedBreakResources));

    MergingRunBreaks = false;

    SWBCHead        = NULL;
    SWBCGarbageHead = NULL;
}
