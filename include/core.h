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

#ifndef HTOS
#define HTOS

#include "ll.h"
#include "cpu/reg.h"
#include "cpu/_irq.h"

extern ll_t core_gc_task;

/// mandatory initialize before any APIs could be used
void core_init(void);

/// start the OS
/// \note this function does not return
void core_start(void);

/// allocate memory block from the start heap
/// \param sz
/// \param align_bits number of bits the required memory should align to
void *core_alloc(unsigned sz, int align_bits);

#define _alloc(_sz)      core_alloc(_sz, 3)

typedef struct {
	unsigned *idles, idle_sz;
	unsigned idle, all;
} core_ut_t;

void core_ut_init(int sample_ticks);

extern core_ut_t core_ut;

extern void (*core_abt) (void *ctx);

typedef struct core_idle {
	lle_t ll;
	void (*notify) (struct core_idle * o);
	void *priv;
} core_idle_t;

void core_idle_listen(core_idle_t * o);

#endif
