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

#include "irq.h"
#include "cfg.h"
#include "soc.h"
#include "dbg.h"
#include "tmr_impl.h"
#include "cpu/reg.h"
#include <string.h>

#if !CFG_IRQ_VECTS
volatile unsigned irq_cnt, irq_depth;

volatile unsigned fiq_cnt;
#endif

irq_t *irqs;

void _irq_init(irq_t * irqs, unsigned irq, irq_t f)
{
	irq_t *o = irqs + (int)irq;
	_assert((int)irq < CFG_IRQ_MAX);
	*o = f;
	if (((int)irq) >= 0)
		irq_unmask(irq);
}

void _irq_dest(irq_t * irqs, unsigned irq)
{
	irq_t *o = irqs + irq;
	*o = 0;
}

void irq_init(unsigned irq, irq_t f)
{
	_irq_init(irqs, irq, f);
}

void irq_dest(unsigned irq)
{
	_irq_dest(irqs, irq);
}

void *irq_bind_data[CFG_IRQ_MAX];

void irq_bind(unsigned irq, void *data)
{
	irq_bind_data[irq] = data;
}

void *irq_data(void)
{
	int n = irq_actn();
	if (n == IRQ_NA)
		return 0;
	return irq_bind_data[n];
}

#if !CFG_IRQ_VECTS

static int irq_act_n;

int irq_actn()
{
	return irq_act_n;
}

int irq_dispatch(irq_sta_t irq, reg_irq_t * reg_irq, int depth)
{
	unsigned r = 0;
	irq_t *o;
	if (irq == IRQ_NA)
		return 0;
	irq_act_n = irq;
#if CFG_TICKLESS
	if (tmr_rtcs)
		tmr_tickless_end();
#endif
	o = irqs + irq;
	if (*o)
		r = (*o) (irq, reg_irq, depth);
	if (r != IRQ_DONE)
		irq_eoi(irq);
	irq_act_n = IRQ_NA;
#if CFG_DBM
	return irq == dbm_irq();
#else
	return 0;
#endif
}
#endif
