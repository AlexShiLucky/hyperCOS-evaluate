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

#ifndef TIMER150423
#define TIMER150423

#include "ll.h"
#include "irq.h"

typedef int (*tmr_do) (void *p);

typedef struct {
	lle_t ll;
	tmr_do f;
	void *p;
	unsigned expire;
	unsigned irq_mode;
} tmr_t;

/// total ticks since boot up
extern unsigned tmr_ticks, tmr_hz;

/// timer irq number
extern unsigned tmr_irq;

/// log2(rtcs_freq/tick_freq)
extern unsigned tmr_rtcs2tick;

tmr_t *tmr_init(tmr_t * t, void *p, tmr_do f);

void _tmr_on(tmr_t * t, unsigned expire, unsigned irq_mode);

static inline void tmr_on(tmr_t * t, unsigned expire)
{
	_tmr_on(t, expire, 0);
}

static inline void tmr_on_irq(tmr_t * t, unsigned expire)
{
	_tmr_on(t, expire, 1);
}

static inline void _tmr_of(tmr_t * t)
{
	lle_del(&t->ll);
}

static inline void tmr_of(tmr_t * t)
{
	unsigned iflag = irq_lock();
	_tmr_of(t);
	irq_restore(iflag);
}

static inline int tmr_active(tmr_t * t)
{
    return t->ll.n != t->ll.p;
}

irq_handler_decl(_tmr_tickf);

#endif
