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

#include "tmr.h"
#include "cfg.h"
#include "task.h"
#include "mq.h"
#include "dbg.h"
#include "io.h"
#include "soc.h"
#include "core.h"
#include "io.h"
#include "sch.h"
#include <string.h>

#define TMR_MQ_SZ       (8*sizeof(unsigned))
#define TMR_ENTS	(1<<CFG_TMR_BITS)

ll_t tmrs[TMR_ENTS];

unsigned tmr_ticks, tmr_irq, tmr_rtcs2tick;

unsigned tmr_hz;

task_t tmr_task;

static mq_t tmr_mq;

tmr_t *tmr_init(tmr_t * t, void *p, tmr_do f)
{
	memset(t, 0, sizeof(*t));
	lle_init(&t->ll);
	t->p = p;
	t->f = f;
	return 0;
}

static inline void _tmr_insert(tmr_t * t, unsigned expire)
{
	unsigned ti;
	t->expire = tmr_ticks + expire;
	//if( expire >= TMR_ENTS )
	//      ti = (tmr_ticks + TMR_ENTS -1);
	//else
	ti = (tmr_ticks + expire) & BI_TMSK(CFG_TMR_BITS);
	ll_addt(tmrs + ti, &t->ll);
}

void _tmr_on(tmr_t * t, unsigned expire, unsigned irq_mode)
{
	unsigned iflag;
	iflag = irq_lock();
	_tmr_insert(t, expire);
	t->irq_mode = irq_mode;
	irq_restore(iflag);
}

static void __tmr_tickf(void)
{
	lle_t *p, *tmp;
	unsigned cur, iflag;
	irq_dep_chk(irq_depth > 0);

	// time slicing
	iflag = irq_lock();
	sch_tick();
	irq_restore(iflag);

	// general timer
	tmr_ticks++;

	cur = tmr_ticks & BI_TMSK(CFG_TMR_BITS);

	ll_for_each_mod(&tmrs[cur], p, tmp) {
		tmr_t *t = lle_get(p, tmr_t, ll);
		int exp;
		if (!t->irq_mode)
			continue;
		exp = t->expire - tmr_ticks;
		if (exp <= 0) {
			exp = t->f(t->p);
			lle_del(&t->ll);
			if (exp != 0)
				_tmr_insert(t, exp);
		}
	}
	if (!ll_empty(&tmrs[cur])) {
		mq_put(&tmr_mq, &cur, WAIT_NO);
	}
}

irq_handler(_tmr_tickf)
{
	__tmr_tickf();
#if !CFG_IRQ_VECTS
	irq_eoi(irq);
#endif
	return IRQ_DONE;
}

static void _tmr_process_slot(int slot)
{
	lle_t *p, *tmp;
	ll_for_each_mod(&tmrs[slot], p, tmp) {
		tmr_t *t = lle_get(p, tmr_t, ll);
		int exp;
		if (t->irq_mode)
			continue;
		exp = t->expire - tmr_ticks;
		if (exp <= 0) {
			exp = t->f(t->p);
			lle_del(&t->ll);
			if (exp != 0)
				_tmr_insert(t, exp);
		}
	}
}

static void tmr_taskf(void *priv)
{
	//unsigned i;
	unsigned cur;

	while (mq_get(&tmr_mq, &cur, WAIT) == 0) {
		//if(cur < TMR_ENTS){
		_tmr_process_slot(cur);
		continue;
		//}
		//for( i = 0 ; i < TMR_ENTS ; i ++)
		//      _tmr_process_slot(i);
	}
}

#if CFG_TICKLESS

unsigned tmr_rtcs;

static int _tmr_next_expire()
{
	tmr_t *t;
	int i, off, noff;

	noff = 0x7fffffff;
	for (i = 0; i < TMR_ENTS; i++) {
		ll_t *l = tmrs + i;
		if (ll_empty(l))
			continue;
		t = lle_get(ll_head(l), tmr_t, ll);
		off = t->expire - tmr_ticks;
		if (off < noff)
			noff = off;
	}
	return noff;
}

int tmr_tickless()
{
	unsigned next_expire;
	unsigned flag = irq_lock();
	irq_mask(tmr_irq);
	tmr_rtcs = soc_rtcs();
	next_expire = _tmr_next_expire();
	tmr_tickless_soc(next_expire);
	irq_restore(flag);
	return next_expire;
}

void tmr_tickless_end()
{
	unsigned rtcs = soc_rtcs();
	unsigned off = (rtcs - tmr_rtcs) >> tmr_rtcs2tick;
	if (off > 1)
		tmr_ticks += off - 1;
	tmr_rtcs = 0;
	__tmr_tickf();
}

#endif

void tmr_init_sys(void)
{
	int i;
	for (i = 0; i < TMR_ENTS; i++)
		ll_init(&tmrs[i]);
	mq_init(&tmr_mq, sizeof(unsigned), _alloc(TMR_MQ_SZ), TMR_MQ_SZ);
	task_init(&tmr_task,
		  "tmr",
		  tmr_taskf,
		  CFG_TMR_PRI, _alloc(CFG_TMR_STACK), CFG_TMR_STACK, -1, 0);
	tmr_irq = tmr_init_soc(&tmr_rtcs2tick, &tmr_hz);
	irq_init(tmr_irq, _tmr_tickf);
}
