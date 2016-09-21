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

#ifndef SOC150408
#define SOC150408

#include "irq.h"

/// APIs provided by SOC back-end and called by OS

unsigned irq_mask(unsigned irq);

void irq_unmask(unsigned irq);

/// \param is_level 0:level triggered, 1:edge triggered
void irq_cfg(unsigned irq, int is_level);

/// \return irq number or IRQ_NA
irq_sta_t irq_ack(void);

#ifndef irq_eoi
/// end of interrupt
void irq_eoi(unsigned irq);
#endif

/// trigger software interrupt
void irq_sgi(unsigned irq);

/// initial a hardware timer to trigger interrupt periodically
/// provided by SOC back-end and called by OS
/// \param [output] log2(rtcs_freq/tick_freq)
/// \param [output] hz
/// \return timer irq
int tmr_init_soc(unsigned *rtcs2tick, unsigned *hz);

/// \return get the Real Time Counter value
unsigned soc_rtcs();

/// \return return the freq of high resolution ticks
unsigned soc_hrt_init(void);

/// \return return high resolution ticks
unsigned soc_hrt(void);

/// \param next_expire left ticks to the next timer event
/// \note the implementation may set wake up time less than <next_expire>
void tmr_tickless_soc(unsigned next_expire);

void tmr_enable(int on);

void soc_idle(int next_expire);

void soc_init(void);

unsigned dbm_irq(void);

int _dbm_get(void);

void dbm_put(char c);

void udelay(int us);

#endif
