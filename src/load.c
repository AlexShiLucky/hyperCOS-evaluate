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
#include "clk.h"
#include "tmr.h"
#include "core.h"
#include "task.h"
#include "load.h"
#include "soc.h"
#include "tmr.h"
#include "sch.h"

#define LOAD_TICKING_SHIFT	5

#define FULL_ITERATION		20

static unsigned load_full;
//SMP:
static load_t core_load;

static clk_t *clk_cpu;

static tmr_t tmr;

static int load_ticking;

static task_switch_t switch_listener;

static void (*tune) (unsigned load, unsigned load_full);

static void sys_tune()
{
	unsigned load = core_load.sum + _task_cur->load.sum;
	if (load > load_full)
		load = load_full;
	if (tune)
		tune(load, load_full);
}

void load_dvfs(unsigned load, unsigned load_full)
{
	int sel = (clk_cpu->freq_n * load) / load_full;
	clk_setf(clk_cpu, sel);
}

static inline unsigned normalize(unsigned load)
{
	return load * clk_cpu->freq[clk_cpu->idx];
}

static inline unsigned _decay(unsigned sum, unsigned rtc_ticks)
{
	unsigned tmr_ticks = rtc_ticks >> tmr_rtcs2tick;
	return (sum >> (tmr_ticks >> LOAD_TICKING_SHIFT));
}

static inline void decay(load_t * load, unsigned now)
{
	load->sum = _decay(load->sum, now - load->ts);
	load->ts = now;
}

static void _switch(task_switch_t * o, task_t * tn)
{
	unsigned now = soc_rtcs();
	if (task_is_idle(_task_cur)) {
		//SMP:
		tmr_on_irq(&tmr, load_ticking);
		_task_cur->load.sum += now - _task_cur->load.ts;
	} else if (clk_cpu) {
		_task_cur->load.sum += normalize(now - _task_cur->load.ts);
	}
	if (task_is_idle(tn)) {
		//SMP:
		tmr_of(&tmr);
		tn->load.ts = now;
	} else if (clk_cpu) {
		if (core_load.sum > tn->load.sum)
			core_load.sum -= tn->load.sum;
		else
			core_load.sum = 0;
		decay(&tn->load, now);
	}
}

static inline int task_switch_active(task_switch_t * o)
{
	return o->notify == _switch;
}

void _wake(task_t * t)
{
	decay(&t->load, soc_rtcs());
	core_load.sum += t->load.sum;
	sys_tune();
}

static int load_tick(void *p)
{
	unsigned iflag = irq_lock();
	unsigned now = soc_rtcs();
	// decay cpu load
	decay(&core_load, now);
	// decay task_cur load
	decay(&_task_cur->load, now);
	// add sample task_cur to avg
	_task_cur->load.sum += normalize(now - _task_cur->load.ts);
	sys_tune();
	irq_restore(iflag);
	return load_ticking;
}

static inline unsigned full_load(clk_t * clk)
{
	unsigned i, sum;
	unsigned max_freq = clk->freq[clk->freq_n - 1];
	unsigned max_sample = load_ticking << tmr_rtcs2tick;
	sum = 0;
	for (i = 0; i < FULL_ITERATION; i++)
		sum = _decay(sum, max_sample) + max_sample * max_freq;
	return sum;
}

void load_init(clk_t * _clk_cpu, int load_ticking_scale, load_tune_f _tune)
{
	clk_cpu = _clk_cpu;
	tune = _tune;
	load_ticking = (load_ticking_scale << LOAD_TICKING_SHIFT);
	load_full = full_load(clk_cpu);
	tmr_init(&tmr, 0, load_tick);
	if (!task_switch_active(&switch_listener)) {
		task_switch_init(&switch_listener, _switch, 0);
		task_switch_listen(&switch_listener);
	}
	sch_wake_notify = _wake;
	core_load.ts = soc_rtcs();
	tmr_on_irq(&tmr, load_ticking);
}

#if CFG_OSUTIL

static tmr_t ut_tmr;

//SMP: cpudata
load_ut_t load_ut;

// call in tmr task context
static int ut_tick(void *p)
{
	unsigned now, iflag;
	task_t *idle = core_idle();
	load_ut_t *ut = &load_ut;
	iflag = irq_lock();
	now = soc_rtcs();
	ut->idle = idle->load.sum;
	ut->all = (now - ut->ut_sta);
	ut->ut_sta = now;
	idle->load.sum = 0;
	irq_restore(iflag);
	return ut->sample_ticks;
}

//SMP: cpuid
void load_ut_init(int sample_ticks)
{
	load_ut_t *ut = &load_ut;
	ut->ut_sta = soc_rtcs();
	ut->sample_ticks = sample_ticks;
	if (!task_switch_active(&switch_listener)) {
		task_switch_init(&switch_listener, _switch, 0);
		task_switch_listen(&switch_listener);
	}
	tmr_init(&ut_tmr, 0, ut_tick);
	tmr_on(&ut_tmr, sample_ticks);
}
#endif
