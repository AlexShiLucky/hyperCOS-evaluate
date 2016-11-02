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

#ifndef TASK150313
#define TASK150313

#include "cfg.h"
#include "ll.h"
#include "wait.h"
#include "tmr.h"
#include "core.h"
#include "irq.h"
#include "dbg.h"
#include "load.h"
#include "cpu/reg.h"
#include "cpu/_cpu.h"

typedef void (*task_ent) (void *priv);

typedef enum {
	TASK_READY,
	TASK_WAIT,
	TASK_DEST,
} task_status_t;

typedef struct task {
	task_ent ent;		///< thread function
	const char *name;
	unsigned *stack;
	int stack_sz;		///< stack size requirements in bytes
	short slice;		///< initial timeslice
	short slice_cur;	///< available time slice before yield
	void *priv;
	lle_t ll;
	reg_t *context;
	tmr_t to;
	char status;		///< task_status_t
	char tm_out;
	short pri;		///< initial thread priority
	load_t load;
} task_t;

extern void (*task_gc) (task_t * t);

/// \arg pri, 1~ (CFG_TPRI_NUM-1), less value has higher priority, 0 is invalid
/// \arg slice, -1 for no time-slice
task_t *task_init(task_t * t,
		  const char *name,
		  task_ent e,
		  int pri,
		  unsigned *stack, int stack_sz, int slice, void *priv);

#define task_new(_n,_e,_p,_sz,_sl,_pr) \
	task_init(_alloc(sizeof(task_t)), _n, _e, _p, _alloc(_sz), _sz, _sl, _pr)

static inline void task_set_idle(task_t * t)
{
	t->load.wei = 0;
}

static inline int task_is_idle(task_t * t)
{
	return t->load.wei == 0;
}

void _task_dest();

extern task_t *_task_cur, *_task_pend;

/// requirecritical section protection
/// \return -1 on timeout
int task_suspend(ll_t * wq, wait_t w);

/// \param w sleep ticks
void task_sleep(wait_t w);

void _task_pri(task_t * t, short pri);

/// \param t task
/// \param pri priority
void task_pri(task_t * t, short pri);

/// \param hint notify the scheduler to bypass thread with
/// priority higher than hint
void sch_schedule(unsigned hint);

static inline void task_pri_self(short pri)
{
	task_t *t = _task_cur;
	short opri = t->pri;
	t->pri = pri;
	if (pri > opri)
		sch_schedule(opri);
}

/// \param yield the control to threads with the same priority
static inline void task_yield()
{
	sch_schedule(_task_cur->pri);
}

int _task_wakeq(ll_t * wq, unsigned iflag);

static inline void task_wakeq(ll_t * wq)
{
	unsigned iflag = irq_lock();
	_task_wakeq(wq, iflag);
}

void _task_load(reg_t * reg_next);

void _task_switch(reg_t * reg_next, reg_t ** reg_cur);

void _task_switch_pend(task_t * tn);

/// \return pointer to original context
reg_t **_task_switch_status(task_t * tn);

extern void (*task_ov) (task_t * t);

#include "cpu/_task.h"

struct task_switch;

typedef struct task_switch task_switch_t;

typedef void (*task_switch_f) (struct task_switch * o, struct task * tn);

struct task_switch {
	lle_t ll;
	task_switch_f notify;
	void *priv;
};

static inline void task_switch_init(task_switch_t * o, task_switch_f notify,
				    void *priv)
{
	lle_init(&o->ll);
	o->notify = notify;
	o->priv = priv;
}

void task_switch_listen(task_switch_t * o);

#endif
