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

#ifndef LL0320
#define LL0320

/// linked list entry
typedef struct lle {
	struct lle *n, *p;
} lle_t;

#define LLE_INIT(le)	{&le, &le}

inline static void lle_init(lle_t * le)
{
	le->n = le->p = le;
}

inline static char *_lle_get(lle_t * le)
{
	return (char *)le;
}

#define lle_get(l, type, mem) \
        ((type *)(_lle_get(l)-(unsigned)(&((type *)0)->mem)))

inline static void lle_add_before(lle_t * le, lle_t * newe)
{
	lle_t *t = le->n;

	le->n = newe;
	le->p = newe;
	newe->n = t;
	newe->p = le;
}

inline static void lle_del(lle_t * le)
{
	le->p->n = le->n;
	le->n->p = le->p;
	le->n = le->p = le;
}

/// linked list head
typedef struct ll {
	lle_t *n, *p;
} ll_t;

#define ll_dec(_l)	ll_t _l = {(void*)&_l, (void*)&_l};

#define ll_for_each(_ll, _lle)		\
	for((_lle) = ll_head(_ll);(_lle) != (void*)(_ll); (_lle) = (_lle)->n)

#define ll_for_each_mod(_ll, _lle, _tmp)	\
	for((_lle) = ll_head(_ll);(_lle) != (void*)(_ll) && ((_tmp)=(_lle)->n);(_lle) = (_tmp))

inline static void ll_init(ll_t * l)
{
	l->n = (void *)l;
	l->p = (void *)l;
}

#define ll_empty(_l)	((_l)->n == (void*)(_l))

inline static void ll_addh(ll_t * l, lle_t * newe)
{
	lle_t *p = l->p;

	p->n = newe;
	l->p = newe;
	newe->p = p;
	newe->n = (void *)l;
}

inline static void ll_addt(ll_t * l, lle_t * newe)
{
	lle_t *t = l->n;

	l->n = newe;
	t->p = newe;
	newe->n = t;
	newe->p = (void *)l;
}

inline static lle_t *ll_head(ll_t * l)
{
	return l->n;
}

#endif
