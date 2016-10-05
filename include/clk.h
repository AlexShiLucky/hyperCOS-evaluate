/*-****************************************************************************/
/*-                                                                           */
/*-            Copyright (c) 2013 by SOCware Inc.                             */
/*-                                                                           */
/*-  This software is copyrighted by and is the sole property of SOCware, Inc.*/
/*-  All rights, title, ownership, or other interests in the software remain  */
/*-  the property of SOCware, Inc. The source code is for FREE short-term     */
/*-  evaluation, educational or non-commercial research only. Any commercial  */
/*-  application may only be used in accordance with the corresponding license*/
/*-  agreement. Any unauthorized use, duplication, transmission, distribution,*/
/*-  or disclosure of this software is expressly forbidden.                   */
/*-                                                                           */
/*-  Knowledge of the source code may NOT be used to develop a similar product*/
/*-                                                                           */
/*-  This Copyright notice may not be removed or modified without prior       */
/*-  written consent of SOCware, Inc.                                         */
/*-                                                                           */
/*-  SOCWare, Inc. reserves the right to modify this software                 */
/*-  without notice.                                                          */
/*-                                                                           */
/*-             socware.help@gmail.com                                        */
/*-             http://socware.net                                            */
/*-                                                                           */
/*-****************************************************************************/
#ifndef CLK1605GG
#define CLK1605GG

#include "ll.h"

typedef enum { CLK_EN, CLK_EN_POST, CLK_SETF, CLK_SETF_POST } clk_evt_t;

struct clk;

typedef struct clk clk_t;

typedef struct clk_listener {
	void (*evt) (struct clk_listener * o, clk_t * clk, clk_evt_t type);
	lle_t ll;
	void *priv;
} clk_listener_t;

struct clk {
	char *name;
	clk_t *parent;
	///< @return 0 on success
	int (*setf) (clk_t * o, int idx);
	///< @return 0 on success
	int (*en) (clk_t * o, int enabled);
	int (*is_en) (clk_t * o);
	unsigned (*getf) (clk_t * o);
	ll_t listeners;
	void *priv;
	int idx;
	unsigned short freq_n;
	unsigned short *freq;
	unsigned short *pwr;
};

static inline int clk_setf(clk_t * o, int idx)
{
	return o->setf(o, idx);
}

static inline int clk_en(clk_t * o, int enabled)
{
	if (!o->en)
		return 0;
	return o->en(o, enabled);
}

static inline int clk_is_en(clk_t * o)
{
	return !o->is_en || o->is_en(o);
}

static inline unsigned clk_getf(clk_t * o)
{
	return o->getf(o);
}

static inline void clk_listen(clk_t * o, clk_listener_t * listener)
{
	ll_addt(&o->listeners, &listener->ll);
}

#endif
