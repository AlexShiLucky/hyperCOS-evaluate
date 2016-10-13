/*-****************************************************************************/
/*-                                                                           */
/*-            Copyright (c) of hyperCOS.                                     */
/*-                                                                           */
/*-  This software is copyrighted by and is the sole property of socware.net. */
/*-  All rights, title, ownership, or other interests in the software remain  */
/*-  the property of socware.net. The source code is FREE for short-term      */
/*-  evaluation, educational or non-commercial research only. Any commercial  */
/*-  application may only be used in accordance with the corresponding license*/
/*-  agreement. Any unauthorized use, duplication, transmission, distribution,*/
/*-  or disclosure of this software is expressly forbidden.                   */
/*-                                                                           */
/*-  Knowledge of the source code may NOT be used to develop a similar product*/
/*-                                                                           */
/*-  This Copyright notice may not be removed or modified without prior       */
/*-  written consent of socware.net.                                          */
/*-                                                                           */
/*-  socware.net reserves the right to modify this software                   */
/*-  without notice.                                                          */
/*-                                                                           */
/*-  To contact socware.net:                                                  */
/*-                                                                           */
/*-             socware.help@gmail.com                                        */
/*-                                                                           */
/*-****************************************************************************/

#ifndef DBG150308
#define DBG150308

#include "cfg.h"
#include "cpu.h"

struct task;
struct reg;

#if CFG_ASSERT == 1
#define _assert(_exp)    if(!(_exp)) _fail(cpu_cur_pc(),0,__LINE__)
#elif CFG_ASSERT == 2
#define _assert(_exp)    if(!(_exp)) _fail(cpu_cur_pc(),__FILE__,__LINE__)
#else
#define _assert(_exp)
#endif

#define _test(_exp)      if(!(_exp)) _fail(cpu_cur_pc(),__FILE__,__LINE__)

int _printf(const char *fmt, ...);

#if _DBG
#define dbg(_fmt, _args...) _printf(_fmt , ##_args)
#else
#define dbg(_fmt, __args...)
#endif

/// provided by APP and is called by OS on fatal error
/// \param addr failure address
/// \param f    source file name
/// \param line source line number
void _fail(void *addr, const char *f, unsigned line);

/// provided by APP and is called when main exits
/// \param the return code
void _exit(int);

/// provided by APP and is called by OS on CPU hardware abort
void _abt(void *ctx);

/// provided by APP and is called by OS when stack overflow is detected
void _stackov(struct task *t);

#endif
