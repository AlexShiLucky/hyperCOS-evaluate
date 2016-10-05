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

#include "core.h"
#include "soc.h"
#include "tmr.h"
#include "io.h"
#include "task.h"
#include "sem.h"
#include "mut.h"
#include "cfg.h"
#include "tmr.h"
#include "tmr_impl.h"
#include "sch.h"
#include "cpu/cache.h"
#include "cpu/_cpu.h"

ll_t core_gc_task;

unsigned core_heap;

static task_t core_task_idle;

static ll_t idle_listeners;

#if CFG_OSUTIL

static unsigned idles[1<<CFG_OSUTIL], iidle;

static unsigned ut_sta;

static tmr_t ut_tmr;

static int sample_ticks;

core_ut_t core_ut = { idles, sizeof(idles) / sizeof(unsigned), 0, 0 };

static int ut_tick(void *p)
{
	unsigned now, iflag;
	iflag = irq_lock();
	now = soc_rtcs();
	core_ut.idles[iidle] = core_task_idle.ut;
	core_task_idle.ut = 0;
	core_ut.idle = (core_ut.idle >> 1) + core_ut.idles[iidle];
	core_ut.all = (core_ut.all >> 1) + (now - ut_sta);
	ut_sta = now;
	irq_restore(iflag);
	iidle = (iidle + 1) & ((1<<CFG_OSUTIL)-1);
	core_ut.idles[iidle] = 0;
	return sample_ticks;
}

void core_ut_init(int _sample_ticks)
{
	ut_sta = soc_rtcs();
	sample_ticks = _sample_ticks;
	tmr_init(&ut_tmr, 0, ut_tick);
	tmr_on(&ut_tmr, _sample_ticks);
}

#endif

static void gcf(core_idle_t * o)
{
	while (!ll_empty(&core_gc_task)) {
		unsigned iflag = irq_lock();
		task_t *gct = lle_get(ll_head(&core_gc_task), task_t, ll);
		lle_del(&gct->ll);
		irq_restore(iflag);
		if (task_gc)
			task_gc(gct);
	}
}

static core_idle_t idle_gc = { LLE_INIT(idle_gc.ll), gcf, 0 };

static void core_idle(void *priv)
{
	lle_t *lle;
	sch_schedule(1);
	for (;;) {
		ll_for_each(&idle_listeners, lle) {
			core_idle_t *l = lle_get(lle, core_idle_t, ll);
			l->notify(l);
		}
#if CFG_TICKLESS
		soc_idle(tmr_tickless());
#else
		cpu_idle();
#endif
	}
}

extern char _end[];

void *core_alloc(unsigned sz, int align_bits)
{
	if (core_heap == 0)
		core_heap = (unsigned)(&_end);
	core_heap = BI_RUP(core_heap, align_bits);
	core_heap += sz;
	return (void *)(core_heap - sz);
}

void core_init()
{
	cache_init();
	cpu_init();
	soc_init();
	sch_init();
	task_init(&core_task_idle,
		  "idle",
		  core_idle,
		  CFG_TPRI_NUM - 1,
		  _alloc(CFG_IDLE_STACK), CFG_IDLE_STACK, 10, 0);
	tmr_init_sys();
	ll_init(&core_gc_task);
	ll_init(&idle_listeners);
#if CFG_TASK_GC
	core_idle_listen(&idle_gc);
#endif
}

void core_start()
{
	lle_del(&core_task_idle.ll);
	_task_cur = &core_task_idle;
	_task_cur->sch = soc_rtcs();
	task_load(&core_task_idle);
}

void core_idle_listen(core_idle_t * o)
{
	ll_addt(&idle_listeners, &o->ll);
}
