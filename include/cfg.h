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

#ifndef CFG150302
#define CFG150302

/// The assert verbose level
/// 0: no assert
/// 1: simple output
/// 2: verbose output
#ifndef CFG_ASSERT
#define CFG_ASSERT	0
#endif

/// support to run in the secure mode, with an optional rich OS e.g. Linux
/// run on top of a non-secure thread
#ifndef CFG_SECURE
#define CFG_SECURE	1
#endif

/// support preemption by none-secure IRQ when running in secure mode
#ifndef CFG_SECURE_NSIRQPRE
#define CFG_SECURE_NSIRQPRE    0
#endif

/// hardware support IRQ vector table
#ifndef CFG_IRQ_VECTS
#define CFG_IRQ_VECTS	0
#endif

/// support OS utilization statistics
#ifndef CFG_FIX_VECT
#define CFG_FIX_VECT	1
#endif

/// required aligned bits for exception vector
#ifndef CFG_VECT_ALIGN
#define CFG_VECT_ALIGN	10
#endif

/// max irq number
#ifndef CFG_IRQ_MAX
#define CFG_IRQ_MAX	128
#endif

/// stack size for IRQ mode
#ifndef CFG_STACK_IRQ
#define CFG_STACK_IRQ	512
#endif

#ifndef CFG_STACK_ABT
#define CFG_STACK_ABT  64
#endif

/// stack size for MON mode
#ifndef CFG_STACK_MON
#define CFG_STACK_MON	1024
#endif

/// stack size of none-secure thread
#ifndef CFG_NS_STACK
#define CFG_NS_STACK	256
#endif

/// stack size for timer thread
#ifndef CFG_TMR_STACK
#define CFG_TMR_STACK	2048
#endif

/// stack size for idle thread,
/// @note the garbage collection may requires more idle stack size.
#ifndef CFG_IDLE_STACK
#define CFG_IDLE_STACK	512
#endif

#ifndef CFG_DBM_STACK
#define CFG_DBM_STACK	2048
#endif

/// support priority queue scheduler
/// 1: linear search
/// 2: bitmap search
#ifndef CFG_SCH_PQ
#define CFG_SCH_PQ	2
#endif

/// max priority number for thread
#ifndef CFG_TPRI_NUM
#define CFG_TPRI_NUM	64
#endif

// enable task garbage collection
#ifndef CFG_TASK_GC
#define CFG_TASK_GC	1
#endif

/// the number of timer slot, in a log2 presentation
/// 1: timer slot = 2^1
/// 2: timer slot = 2^2 = 4
/// ...
/// 5: timer slot = 2^5 = 32
#ifndef CFG_TMR_BITS
#define CFG_TMR_BITS	5
#endif

/// support nested IRQ handler
#ifndef CFG_INT_NESTED
#define CFG_INT_NESTED	1
#endif

#ifndef CFG_INT_NESTED_MAX
#define CFG_INT_NESTED_MAX	6
#endif

/// cache line size
#ifndef CFG_CACHE_LSZ
#define CFG_CACHE_LSZ	64
#endif

/// timer thread priority
#ifndef CFG_TMR_PRI
#define CFG_TMR_PRI	1
#endif

/// memory block poll use div instruction
#ifndef CFG_MEMP_DIV
#define CFG_MEMP_DIV	0
#endif

/// use ARM architecture version
/// 5:  armv5
/// 7:  armv7
///
#ifndef CFG_AARCH
#define CFG_AARCH	7
#endif

/// use hardfloat
#ifndef CFG_HFLOAT
#define CFG_HFLOAT 1
#endif

/// use ARM architecture version
#ifndef CFG_CACHE_VMSA
#define CFG_CACHE_VMSA 1
#endif

/// use ARM architecture version
#define CFG_CACHE_PMSA (!CFG_CACHE_VMSA)

/// support tickless behavior
#ifndef CFG_TICKLESS
#define CFG_TICKLESS	1
#endif

/// support OS utilization statistics
#ifndef CFG_OSUTIL
#define CFG_OSUTIL	3
#endif

/// support stack overflow detection
#ifndef CFG_STACK_CHK
#define CFG_STACK_CHK	1
#endif

#define CFG_TZ		(CFG_AARCH>=6)

#if CFG_AARCH<6 && defined(__thumb__)
#define CFG_ASM_STUB	1
#else
#define CFG_ASM_STUB	0
#endif

#define CFG_ASM_UNIFY	(CFG_AARCH>=6)

#define CFG_ASM_WFI	(CFG_AARCH>=7)

#define CFG_ASM_BFI	(CFG_AARCH>=7)

#define CFG_ASM_RFE	(CFG_AARCH>=7)

#ifndef CFG_DBM
#define CFG_DBM         0
#endif

#if CFG_DBM && CFG_INT_NESTED
#error "DBM conflict with INT_NESTED"
#endif

#if CFG_DBM && CFG_AARCH >= 7
#error "DBM does not support v7"
#endif

#ifndef CFG_MAXBP
#define CFG_MAXBP       8
#endif

#ifndef DBG_ELF_CPU
#define	 DBG_ELF_CPU	0
#endif

#ifndef DBG_MOD
#define DBG_MOD		0
#endif

#ifndef DBG_DBM
#define DBG_DBM		0
#endif

#ifndef DBG_TASK
#define DBG_TASK	0
#endif

#ifndef DBG_DBM_ARM
#define DBG_DBM_ARM	0
#endif

#ifndef DBG_CPUM
#define DBG_CPUM	0
#endif

#endif
