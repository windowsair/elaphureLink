/**************************************************************************/ /**
 *           Cortex-M Middle/Upper layer Debug driver Template for µVision
 *
 * @version  V1.0.2
 * @date     $Date: 2017-09-20 19:31:21 +0200 (Wed, 20 Sep 2017) $
 *
 * @note
 * Copyright (C) 2009-2016 ARM Limited. All rights reserved.
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



#ifndef __Flash_H__
#define __Flash_H__


#include "Flash\FlashOS.h"


#define NFlash 10 // maximum number of Flash Devices

struct FlashAlgorithm {
    DWORD PrgBuf;       // Program Buffer
    DWORD BreakPoint;   // BreakPoint
    DWORD Init;         // Init Function
    DWORD UnInit;       // UnInit Function
    DWORD CalculateCRC; // Calculate CRC Function
    DWORD BlankCheck;   // Blank Check Function
    DWORD EraseChip;    // EraseChip Function
    DWORD EraseSector;  // EraseSector Function
    DWORD ProgramPage;  // ProgramPage Function
    DWORD Verify;       // Verify Function
    DWORD rSB;          // Static Base
    DWORD rSP;          // Stack Pointer
    DWORD StackSize;    // Stack Size
};

extern struct FlashDevice    FlashDev; // Flash Device Description Info
extern struct FlashAlgorithm FlashAlg; // Flash Device Algorithm Info


// Flash Options
#define FLASH_ERASE    0x0001
#define FLASH_PROGRAM  0x0002
#define FLASH_VERIFY   0x0004
#define FLASH_RESETRUN 0x0008
#define FLASH_ERASEALL 0x0010


// Flash Configuration
struct FLASHCONF {
    DWORD Opt;      // Flash Options
    DWORD RAMStart; // RAM Start for Flash Functions
    DWORD RAMSize;  // RAM Size for Flash Functions  // 11.9.2017
    WORD  Nitems;   // Number of Flash types
    struct {
        char  FileName[32];        // Flash File Name (without extension)
        char  DevName[128];        // Device Name
        WORD  DevType;             // Device Type
        DWORD Start;               // Start Address of the Flash
        DWORD Size;                // Size of the Flash
        char  fPath[MAX_PATH + 2]; // Device-algo fullpath (RTE)  /7.11.2012/
    } Dev[NFlash];                 // Flash Device
};
extern struct FLASHCONF FlashConf;


extern BOOL LoadFlashDevice(char *fname);
extern BOOL LoadFlashAlgorithm(char *fname);

extern DWORD InitDevFlash(void);
extern DWORD FlashLoad(void);
extern DWORD EraseFlash(void);

extern void InitFlash(void); // Initialize module variables
#endif
