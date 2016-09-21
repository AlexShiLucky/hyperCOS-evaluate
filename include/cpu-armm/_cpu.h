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

#ifndef CPU150822
#define CPU150822

#include "cfg.h"
#include "reg.h"
#include "asm.h"
#include "../dbg.h"
#include "../io.h"

static inline void cpu_idle()
{
	asm volatile ("wfi":::"r0");
}

static inline void cpu_stick_init(unsigned ticks)
{
	reg(STICK + 0x4) = ticks - 1;
	reg(STICK + 0x8) = 0;
	reg(STICK + 0x0) = 0x7;
	// [2]internal-src| [1]enable-int| [0]kick-off
}

static inline unsigned cpu_stick_read()
{
	return readl(STICK + 0x8);
}

static inline void cpu_stick_en(unsigned on)
{
	reg(STICK + 0x0) = on ? 0x7 : 0x4;
}

static inline void cpu_req_switch()
{
	reg(ICSR) |= (1 << 28);
	asm_mb();
}

static inline unsigned cpu_fpu_sta()
{
	return reg(CPACR) & (0xF << 20);
}

static inline void cpu_fpu_en(unsigned sta)
{
	reg(CPACR) &= ~(0xF << 20);
	reg(CPACR) |= (sta << 20);
	asm_mb();
}

#define cpu_fpu_on()	cpu_fpu_en(0xF)

typedef enum {
	E_NMI = 2,
	E_HARD = 3,
	E_MEM = 4,
	E_BUS = 5,
	E_USE = 6,
	E_SVC = 11,
	E_PENDSV = 14,
	E_STICK = 15,
	E_INT = 16,
} cpu_exc_t;

extern unsigned cpu_pbits;

#define cpu_pri_max()		((1<<cpu_pbits)-1)

#define cpu_pri_reg(exc)	regb(SHPR + (exc-4))

static inline void cpu_pri(cpu_exc_t exc, unsigned pri)
{
	_assert(pri <= cpu_pri_max());
	cpu_pri_reg(exc) = pri << (8 - cpu_pbits);
}

void cpu_init(void);

static inline void cpu_abt()
{
	asm volatile ("svc 0");
	asm volatile ("mov r1, r1");
}

#endif
