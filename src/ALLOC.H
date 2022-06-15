/**************************************************************************//**
 *           ALLOC.H: Memory Environment Definitions
 *
 * @version  V1.0.0
 * @date     $Date: 2015-04-28 15:09:20 +0200 (Tue, 28 Apr 2015) $
 *
 * @note
 * Copyright (C) 1999-2009 KEIL, 2009-2015 ARM Limited. All rights reserved.
 *
 * @par
 * ARM Limited (ARM) is supplying this software for use with Keil uVision.
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

#ifndef ALLOC_INCED
  #define ALLOC_INCED


#ifdef __cplusplus
  extern "C"  {
#endif


#include "ComTyp.h"



#define MCU      struct mcu
#define MENV     struct menv
#define MCUFP    struct mcu  *
#define MCC      4096

struct mcu  {
  MCUFP      next;                  /* Link to next Cluster */
  DWORD      aval;
  BYTE     b[MCC];
};

struct menv  {
  MCUFP      head;
  MCUFP      tail;
};

#define ENV_LVT   0       /* Mem-Env for over all LifeTime of uv3 */
#define ENV_PRJ   1       /* memory for Project */
#define ENV_TMP   2       /* temporary Environment */
#define ENV_DBB   3       /* DataBase Environment */
#define ENV_DLG   4       /* temporary for link/loc dialog pages */
#define ENV_DTP   5       /* temporary Mem-Env for Debug */
#define ENV_DEB   6       /* Mem-Env for Debug-Session (syms, types) */
#define ENV_DBM   7       /* Mem-Env for Watch-lists, MemSegs, AttrSegs */
#define ENV_LWA   8       /* Mem-Env for Local Watches */
#define ENV_EDB   9       /* Mem-Env for Ed-breaks etc. */
#define ENV_FIF   10      /* Find-in-Files environment */
#define ENV_FTMP  11      // FuncGen temporary environment
#define ENV_PCOD  12      // FuncGen 'pcode' environment
#define ENV_PP    13      // Preproc-Env (ARM-ADS Dependency)
#define ENV_FCMD  14      // File-Cmd (Project-File Nodes)
#define ENV_RTAMM 15      // RealTimeAgent DCC-buffering

//#define NENV  (ENV_FCMD + 1)   /* Number of Environments */
#define NENV  (ENV_RTAMM + 1)   /* Number of Environments */


extern void         FreeAll (void);
extern void         FreeEnv (DWORD env);
extern void         *GetMem (DWORD many, int env);
extern char     *SaveString (char *s, int env);


#ifdef __cplusplus
  }          /* end extern "C"  { */
#endif


#endif
