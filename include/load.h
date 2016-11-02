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
#ifndef LOAD0930
#define LOAD0930

#include "clk.h"
#include <string.h>

typedef struct {
	unsigned ts;		///< lastest scheduled time stamp
	unsigned sum;
	unsigned wei;		///< weight, idle task shall be 0
} load_t;

typedef void (*load_tune_f) (unsigned load, unsigned load_full);

/// @param load_ticking_scale, set 1 to sync every 32ticks, set 2 to sync every
/// 64 ticks and so on.
void load_init(clk_t * cpu_clk, int load_ticking_scale, load_tune_f tune);

void load_dvfs(unsigned load, unsigned load_full);

typedef struct {
	unsigned idle, all;
	unsigned ut_sta;
	int sample_ticks;
} load_ut_t;

void load_ut_init(int sample_ticks);

extern load_ut_t load_ut;

#endif
