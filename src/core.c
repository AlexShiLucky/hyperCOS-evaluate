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

//SMP: cpudata
static task_t core_task_idle;

static void _idle(void *priv)
{
	sch_schedule(1);
	for (;;) {
#if CFG_TASK_GC
		while (!ll_empty(&core_gc_task)) {
			unsigned iflag = irq_lock();
			task_t *gct =
			    lle_get(ll_head(&core_gc_task), task_t, ll);
			lle_del(&gct->ll);
			irq_restore(iflag);
			if (task_gc)
				task_gc(gct);
		}
#endif
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
	tmr_init_sys();
	ll_init(&core_gc_task);

	//SMP: per-cpu init
	task_init(&core_task_idle,
		  "idle",
		  _idle,
		  CFG_TPRI_NUM - 1,
		  _alloc(CFG_IDLE_STACK), CFG_IDLE_STACK, 10, 0);
	task_set_idle(&core_task_idle);
}

void core_start()
{
	lle_del(&core_task_idle.ll);
	_task_cur = &core_task_idle;
	_task_cur->load.ts = soc_rtcs();
	//SMP: bootup
	task_load(&core_task_idle);
}

struct task *core_idle()
{
	return &core_task_idle;
}
