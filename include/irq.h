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

#ifndef IRQ150303
#define IRQ150303

#include "cfg.h"
#include "cpu/_irq.h"

/// the irq vector
extern irq_t *irqs;

/// number of IRQs after bootup
extern volatile unsigned irq_cnt;

/// the nested depth of IRQ
extern volatile unsigned irq_depth;

void irq_init(unsigned irq, irq_t f);

void irq_dest(unsigned irq);

void irq_bind(unsigned irq, void *data);

void *irq_data(void);

/// @return the active irq number or IRQ_NA
int irq_actn();

#endif
