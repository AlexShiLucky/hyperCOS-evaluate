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

sem_t *sem_init(sem_t * s, int init_val)
{
	memset(s, 0, sizeof(*s));
	ll_init(&s->wait);
	s->val = init_val;
	return s;
}

int sem_get_wait(sem_t * s, wait_t w, unsigned iflag)
{
	if (task_suspend(&s->wait, w)) {
		irq_restore(iflag);
		return -1;
	} else {
		_assert(s->val > 0);
		s->val--;
		irq_restore(iflag);
		return 0;
	}
}

void sem_post_wake(sem_t * s)
{
	task_wakeq(&s->wait);
}

int sem_post(sem_t * s)
{
    return sem_post_n(s, 1);
}
