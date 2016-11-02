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

#include "task.h"
#include "irq.h"
#include "wait.h"
#include "sch.h"
#include "dbg.h"
#include "soc.h"
#include "core.h"
#include "cpu/_cpu.h"
#include <string.h>

reg_t *_cpu_task_init(struct task *t, void *priv, void *dest);

task_t *_task_cur, *_task_pend;

void (*task_gc) (task_t * t);

static ll_t switch_listeners = LL_INIT(switch_listeners);

static int task_wait_timeout(void *p)
{
	task_t *t = (task_t *) p;
	unsigned iflag = irq_lock();
	t->tm_out |= 1;
	if (t->status != TASK_READY) {
		lle_del(&t->ll);
		sch_wake(t);
	}
	t->load.wei = 1;
	irq_restore(iflag);
	return 0;
}

void _task_dest(task_t * t)
{
	unsigned iflag;
	iflag = irq_lock();
	if (t->status == TASK_READY)
		sch_del(t);
	t->status = TASK_DEST;
	ll_addt(&core_gc_task, &t->ll);
	irq_restore(iflag);
}

static void task_dest()
{
	_task_dest(_task_cur);
	task_yield();
}

task_t *task_init(task_t * t,
		  const char *name,
		  task_ent e,
		  int pri, unsigned *stack, int stack_sz, int slice, void *priv)
{
	unsigned iflag;
	reg_t *r;
	memset(t, 0, sizeof(*t));
	t->name = name;
	t->ent = e;
	t->pri = pri;
	t->stack = stack;
	t->stack_sz = stack_sz;
	t->slice = slice;
	t->slice_cur = slice;
	iflag = irq_lock();
	tmr_init(&t->to, t, task_wait_timeout);

	_assert(pri != 0);
	_assert(pri < CFG_TPRI_NUM);

	r = _cpu_task_init(t, priv, task_dest);
	t->context = r;
	lle_init(&t->ll);
	t->status = TASK_READY;
	sch_add(t);
#ifdef CFG_STACK_CHK
	*stack = 0xABBA;
#endif
	irq_restore(iflag);

	if (_task_cur && _task_cur->pri > t->pri)
		task_yield();
	return t;
}

/// requirecritical section protection
int task_suspend(ll_t * wq, wait_t w)
{
	int f;
	lle_t *p;
	task_t *tc = _task_cur;

	irq_dep_chk(irq_depth == 0);
	if (w == WAIT_NO)
		return -1;

	ll_for_each(wq, p) {
		task_t *t = lle_get(p, task_t, ll);
		if (t->pri > tc->pri)
			break;
	}
	lle_add_before(p, &tc->ll);

	tc->status = TASK_WAIT;
	if (w != WAIT) {
		tmr_on(&tc->to, (unsigned)w);
	}
	tc->tm_out &= ~1;
	task_yield();
	_tmr_of(&tc->to);
	f = tc->tm_out & 1;
	return f;
}

int _task_wakeq(ll_t * wq, unsigned iflag)
{
	if (!ll_empty(wq)) {
		task_t *t = lle_get(ll_head(wq), task_t, ll);
		lle_del(&t->ll);
		sch_wake(t);
		irq_restore(iflag);
		return 0;
	} else {
		irq_restore(iflag);
		return 1;
	}
}

void task_sleep(wait_t w)
{
	task_t *tc = _task_cur;
	unsigned iflag;
	iflag = irq_lock();
	irq_dep_chk(irq_depth == 0);
	if (w) {
		tc->status = TASK_WAIT;
		tmr_on(&tc->to, (unsigned)w);
	}
	task_yield();
	irq_restore(iflag);
}

void _task_pri(task_t * t, short pri)
{
	t->pri = pri;
	if (t->status == TASK_READY) {
		lle_del(&t->ll);
		sch_wake(t);
	}
}

void task_pri(task_t * t, short pri)
{
	unsigned iflag;

	irq_dep_chk(irq_depth == 0);
	_assert(t != _task_cur);

	iflag = irq_lock();
	_task_pri(t, pri);
	irq_restore(iflag);
}

reg_t **_task_switch_status(task_t * tn)
{
	lle_t *lle;
	task_t *t = _task_cur;
	if (_task_cur->status == TASK_READY)
		sch_add(_task_cur);
	sch_del(tn);

	ll_for_each(&switch_listeners, lle) {
		task_switch_t *l = lle_get(lle, task_switch_t, ll);
		l->notify(l, tn);
	}

	_task_cur = tn;
	_task_cur->slice_cur = _task_cur->slice;
	return &t->context;
}

void _task_switch_pend(task_t * tn)
{
	irq_dep_chk(irq_depth > 0);
	if (!_task_pend || _task_pend->pri >= tn->pri) {
		_task_pend = tn;
		cpu_req_switch();
	}
}

void task_switch_listen(task_switch_t * o)
{
	ll_addt(&switch_listeners, &o->ll);
}
