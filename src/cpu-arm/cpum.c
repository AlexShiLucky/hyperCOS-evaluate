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
#include "cfg.h"
#define _DBG	DBG_CPUM
#include "dbg.h"
#include "task.h"
#include "io.h"
#include "cpu-armm/asm.h"
#include "cpu-armm/_cpu.h"
#include "cpu-armm/_irq.h"
#include "cpu-armm/reg.h"
#include "cpu-armm/nvic.h"
#include "../tmr_impl.h"
#include <string.h>

extern void __abt(void);

extern void _hfault(void);

extern void _task_pendsv(void);

extern unsigned _stack[];

int fctx_corrupted;

unsigned cpu_pbits;

static inline void *stack_top(task_t * t)
{
	return (((char *)t->stack) + t->stack_sz);
}

#if CFG_TICKLESS && !CFG_FIX_VECT
#error "CFG_TICKLESS requires CFG_FIX_VECT"
#endif

#if CFG_FIX_VECT
void *__isr_vector[E_INT + CFG_IRQ_MAX];

irq_sta_t __irq()
{
	irq_t _irq;
	int n = irq_act();
#if CFG_TICKLESS
	if (tmr_rtcs)
		tmr_tickless_end();
#endif
	_irq = (irq_t) __isr_vector[n];
	if (_irq)
		return _irq();
	return IRQ_DONE;
}
#endif

static irq_sta_t _halt()
{
	while (1) ;
	return 0;
}

int irq_actn()
{
	int n = irq_act();
	return n >= E_INT ? (n - E_INT) : IRQ_NA;
}

void **_vects;

void cpu_init()
{
	void *msp;
	int i;
	int n = nvic_irqn();
	int sz = (E_INT + n) * sizeof(void *);
	if (n > CFG_IRQ_MAX)
		_fail(cpu_cur_pc(), 0, __LINE__);
	// aligned vect
	_vects = (void **)core_alloc(sz, CFG_VECT_ALIGN);
	// 16-bytes aligned msp
	msp = (void **)core_alloc(CFG_STACK_IRQ, 4);
	for (i = 1; i < E_INT; i++)
		_vects[i] = _halt;
	_vects[E_NMI] = __abt;
	_vects[E_HARD] = _hfault;
	_vects[E_MEM] = __abt;
	_vects[E_BUS] = __abt;
	_vects[E_SVC] = __abt;
	_vects[E_PENDSV] = _task_pendsv;
#if CFG_FIX_VECT
	_vects[E_STICK] = __irq;
	for (i = 0; i < n; i++)
		_vects[E_INT + i] = __irq;
	irqs = (irq_t *) (&__isr_vector[E_INT]);
#else
	irqs = (irq_t *) (&_vects[E_INT]);
#endif
	for (i = 0; i < n; i++)
		irqs[i] = _halt;
	reg(VTOR) = (unsigned)_vects;

	// 4-bytes aligned exception stack
	reg(CCR) &= ~(1 << 9);
	// allow hardfault recover
	reg(CCR) |= (1 << 8);

	//SP_main = _stack &
	//SP_process enable
	asm volatile (""
		      "mov r1, sp\n"
		      "msr MSP, %0\n"
		      "mov r2, #2\n"
		      "msr CONTROL, r2\n" "mov sp, r1\n"::"r" (msp)
		      :"r1", "r2");
	cpu_fpu_on();
	cpu_pri_reg(E_SVC) = 0xff;
	cpu_pbits = 9 - __builtin_ffs(cpu_pri_reg(E_SVC));
	cpu_pri(E_SVC, cpu_pri_max());
	cpu_pri(E_PENDSV, cpu_pri_max());
}

void *cpu_cur_pc(unsigned line)
{
	return __builtin_return_address(0);
}

reg_t *_cpu_task_init(task_t * t, void *priv, void *dest)
{
	int sz = sizeof(reg_f_t) + sizeof(reg_t);
	reg_t *r = stack_top(t) - sz;
	memset(r, 0, sz);
	r->eret = 0xFFFFFFFD;
	r->r0 = (unsigned)priv;
	r->lr = (unsigned)dest;
	r->pc = (unsigned)t->ent;
	r->cpsr = ((r->pc & 0x1) << 24);
	return (reg_t *) r;
}

///< get float context soft frame
static inline reg_f_t *_fctx_sf(task_t * t)
{
	return (reg_f_t *) (stack_top(t) - sizeof(reg_f_t));
}

///< get float context soft frame
static inline reg_fhard_t *_fctx_hf(task_t * t)
{
	return &(_fctx_sf(t)->h);
}

#if CFG_HFLOAT
static task_t *cpu_fctx;

int cpu_tf(reg_irq_t * ctx)
{
	reg_fhard_t *fhard;
	reg_f_t *fsoft;
	unsigned fpscr, ipsr;

	if (!(reg(0xE000ED28) & 0x80000))
		return 0;
	if (reg(CPACR) & (0xF << 20))
		return 0;
	ipsr = ctx->cpsr & 0x1ff;

	cpu_fpu_on();
	dbg("FF:%s %s %x %d\r\n", _task_cur->name,
	    cpu_fctx ? cpu_fctx->name : "", ipsr, fctx_corrupted);
	if (!ipsr && cpu_fctx == _task_cur) {
		if (fctx_corrupted) {
			fhard = _fctx_hf(_task_cur);
			asm volatile ("ldr  %0, [%1, #4]\n"
				      "vmsr FPSCR, %0\n":"=r" (fpscr)
				      :"r"(fhard)
				      :);
			asm volatile ("vldmia %0, {s0-s15}"::"r" (fhard->s)
				      :);
			fctx_corrupted = 0;
		}
		return 1;
	}
	// by ISR
	// by Thread && cpu_fctx != _task_cur
	if (cpu_fctx && !fctx_corrupted) {
		//stop  = stack_top(cpu_fctx);
		//fhard = (reg_fhard_t*)(stop - sizeof(reg_fhard_t));
		fhard = _fctx_hf(cpu_fctx);
		asm volatile ("vmrs    %0, FPSCR\n"
			      "str     %0, [%1, #4]\n":"=r" (fpscr)
			      :"r"(fhard)
			      :);
		asm volatile ("vstmia %0, {s0-s15}"::"r" (fhard->s)
			      :);
		fctx_corrupted = 1;
	}

	if (ipsr) {
		//turn off FPU after ISR
		cpu_req_switch();
		return 1;
	}
	// by Thread && cpu_fctx != _task_cur
	if (cpu_fctx) {
		//stop = stack_top(cpu_fctx);
		//fsoft= stop - sizeof(reg_f_t);
		fsoft = _fctx_sf(cpu_fctx);
		//save s16~s31 @ cpu_fctx
		asm volatile ("vstmia %0, {s16-s31}"::"r" (fsoft)
			      :);
	}
	//load s16~s31
	//stop = stack_top(_task_cur);
	//fsoft= stop - sizeof(reg_f_t);
	fsoft = _fctx_sf(_task_cur);
	asm volatile ("vldmia %0, {s16-s31}"::"r" (fsoft)
		      :);
	//load s0~s16 @ _task_cur
	//fhard= (reg_fhard_t*)(stop - sizeof(reg_fhard_t));
	fhard = _fctx_hf(_task_cur);
	asm volatile ("ldr  %0, [%1, #4]\n" "vmsr FPSCR, %0\n":"=r" (fpscr)
		      :"r"(fhard)
		      :);
	asm volatile ("vldmia %0, {s0-s15}"::"r" (fhard->s)
		      :);
	fctx_corrupted = 0;
	cpu_fctx = _task_cur;
	return 1;
}
#else
int cpu_tf(reg_irq_t * ctx)
{
	return 0;
}
#endif

char *cpu_reg_names[] = {
	"r4", "r5", "r6", "r7",
	"r8", "r9", "r10", "r11",
	"r0", "r1", "r2", "r3",
	"r12", "lr", "pc", "xpsr"
};
