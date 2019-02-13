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

#include "sem.h"
#include "ll.h"
#include "task.h"
#include "irq.h"
#include <string.h>

int sem_get_wait(sem_t * s, wait_t w, unsigned iflag);

void sem_post_wake(sem_t * s);

/// \return 0 on success
int sem_get(sem_t * s, wait_t w)
{
	unsigned iflag = irq_lock();
	if (s->val > 0) {
		s->val--;
		irq_restore(iflag);
		return 0;
	}
	return sem_get_wait(s, w, iflag);
}

int sem_post(sem_t * s)
{
	unsigned iflag = irq_lock();
	s->val++;
	if (s->val > 0) {
		irq_restore(iflag);
		sem_post_wake(s);
	} else {
		irq_restore(iflag);
	}
	return 0;
}
