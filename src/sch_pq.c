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
#include "task.h"
#include "io.h"

void (*sch_wake_notify) (struct task * t);

static ll_t task_ready[CFG_TPRI_NUM];

#if CFG_SCH_PQ == 1

void sch_schedule(unsigned hint)
{
	task_t *tn;
	unsigned i, iflag;

	_assert(_task_cur != 0);
	irq_dep_chk(irq_depth == 0);

//L_restart:
//      irq_prev = irq_cnt;
	iflag = irq_lock();
	for (i = hint; i < CFG_TPRI_NUM; i++) {

		if (!ll_empty(&task_ready[i])) {

			tn = lle_get(ll_head(&task_ready[i]), task_t, ll);
			_assert(tn != _task_cur);
//          this might inprove RT but may has starvation problem when system busy
//                      iflag = irq_lock();
//                      if(irq_prev != irq_cnt){
//                              irq_prev = irq_cnt;
//                              irq_restore(iflag);
//                              goto L_restart;
//                      }
			if ((_task_cur->status != TASK_READY) ||
			    (tn->pri < _task_cur->pri)) {

				_task_switch_sync(tn, _task_switch_status(tn));
			}

			irq_restore(iflag);
			return;
		}
	}
	irq_restore(iflag);
	_assert(0);
}

void sch_add(task_t * t)
{
	ll_addt(&task_ready[t->pri], &t->ll);
}

void sch_del(task_t * t)
{
	lle_del(&t->ll);
}

#elif CFG_SCH_PQ == 2
#include <string.h>

unsigned map_l1[BI_RUP(CFG_TPRI_NUM, 10) >> 10];

unsigned map_l2[BI_RUP(CFG_TPRI_NUM, 5) >> 5];

void sch_schedule(unsigned hint)
{
	task_t *tn;
	unsigned i, iflag;

	_assert(_task_cur != 0);
	irq_dep_chk(irq_depth == 0);

	iflag = irq_lock();

	for (i = 0; i < sizeof(map_l1) / sizeof(unsigned); i++) {

		unsigned m, n, q;
		m = __builtin_ffs(map_l1[i]);
		if (!m)
			continue;
		m--;

		n = __builtin_ffs(map_l2[m]);
		_assert(n != 0);
		n--;

		q = (m << 5) + n;
		_assert(!ll_empty(&task_ready[q]));

		tn = lle_get(ll_head(&task_ready[q]), task_t, ll);
		_assert(tn != _task_cur);

		if ((_task_cur->status != TASK_READY) ||
		    (tn->pri < _task_cur->pri)) {

			_task_switch_sync(tn, _task_switch_status(tn));
		}
		irq_restore(iflag);
		return;
	}
	irq_restore(iflag);
	_assert(0);
}

void sch_add(task_t * t)
{
	unsigned *l2, *l1;

	l2 = map_l2 + (t->pri >> 5);
	*l2 = BI_O_BIT(*l2, t->pri & 0x1f);
	l1 = map_l1 + (t->pri >> 10);
	*l1 = BI_O_BIT(*l1, ((t->pri >> 5) & 0x1f));
	ll_addt(&task_ready[t->pri], &t->ll);
}

void sch_del(task_t * t)
{
	lle_del(&t->ll);
	if (ll_empty(&task_ready[t->pri])) {

		unsigned *l2, *l1;

		l2 = map_l2 + (t->pri >> 5);
		*l2 = BI_Z_BIT(*l2, t->pri & 0x1f);

		if (*l2 == 0) {
			l1 = map_l1 + (t->pri >> 10);
			*l1 = BI_Z_BIT(*l1, ((t->pri >> 5) & 0x1f));
		}
	}
}

#else
#error "no scheduler config"
#endif

void sch_tick(void)
{
	task_t *tn, *tc;

	tc = _task_cur;

	if (tc->slice_cur > 0)
		tc->slice_cur--;
	if (tc->slice_cur == 0 && !ll_empty(&task_ready[tc->pri])) {

		tn = lle_get(ll_head(&task_ready[tc->pri]), task_t, ll);
		_task_switch_pend(tn);
	}
}

void sch_wake(task_t * t)
{
	t->status = TASK_READY;
	sch_add(t);
	if (t->pri < _task_cur->pri) {
		if (irq_act())
			_task_switch_pend(t);
		else
			_task_switch_sync(t, _task_switch_status(t));
	} else {
		if (sch_wake_notify)
			sch_wake_notify(t);
	}
}

void sch_init()
{
	int i;
	for (i = 1; i < CFG_TPRI_NUM; i++)
		ll_init(&task_ready[i]);
}
