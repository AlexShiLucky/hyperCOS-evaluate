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

#include "asm-off.h"
#include "cpu/_irq.h"

	.asm_syn
	.text
	.asm_fun sem_get
sem_get:
	ldrex  r2, [r0, #sem_val]
	add    r2, r2, #-1
	cmp    r2, #0
	blt    1f
	strex  r3, r2, [r0]
	cmp    r3, #0
	bne    sem_get
	mov    r0, #0
	bx     lr
1:
	.irq_lock r2, r3
	// FIXME: give up ldrex mux
	ldr    r3, [r0, #sem_val]
	cmp    r3, #0
	ble    sem_get_wait
	.irq_restore r2
	b      sem_get

	.asm_fun sem_post
sem_post:
	ldrex  r2, [r0, #sem_val]
	add    r2, r2, r1
	strex  r3, r2, [r0]
	cmp    r3, #0
	bne    sem_post
	cmp    r2, #0
	bgt    sem_post_wake
	mov    r0, #0
	bx     lr

