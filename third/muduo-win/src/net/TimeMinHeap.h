//////////////////////////////////////////////////////////////////////////
//
// Copyright(C)	2016, gjm_kevin.  All rights reserved.
// Author	:	gjm_kevin@163.com
//
//////////////////////////////////////////////////////////////////////////

#ifndef CALM_NET_TIMEMINHEAP_H_
#define CALM_NET_TIMEMINHEAP_H_


#include "Timer.h"
#include <malloc.h>

#define GET_LEFT_CHILD(i)   ((i*2)+1)
#define GET_RIGHT_CHILD(i)  ((i+1)*2)
#define GET_PARENT(i) ((i-1)/2)
using namespace muduo;
using namespace muduo::net;

typedef struct min_heap
{
	Timer** event ;
	//void ** value;
	unsigned size;   // num of heap
	unsigned capacity;
} min_heap_t;

typedef int(*comp_func)(void*, void*);

static inline void	min_heap_ctor(min_heap_t* s);
static inline void	min_heap_dtor(min_heap_t* s);
static inline void	min_heap_elem_init(Timer* e);
static inline int	min_heap_push(min_heap_t* s,Timer* ele);
static inline Timer* min_heap_pop(min_heap_t* s);
static inline int min_heap_erase(min_heap_t*s, Timer* e);
static inline int	min_heap_reserve(min_heap_t* s, unsigned n);
static inline void	min_heap_shift_up_(min_heap_t* s, unsigned hole_index, Timer* e);
static inline void	min_heap_shift_down_(min_heap_t* s, unsigned hole_index, Timer* e);

static inline int	     min_heap_elt_is_top(const Timer *e);
static inline int	     min_heap_empty(min_heap_t* s);
static inline unsigned	     min_heap_size(min_heap_t* s);
static inline Timer*  min_heap_top(min_heap_t* s);
static inline int compare_int_keys(register int64_t key1, register int64_t key2);
static inline int min_heap_elem_greater(Timer *lhs, Timer *rhs);

int compare_int_keys(register int64_t key1, register int64_t key2) {
	
	// Perform the comparison
	if (key1 < key2)
		return -1;
	else if (key1 == key2)
		return 0;
	else
		return 1;
}


int min_heap_elem_greater(Timer *lhs, Timer *rhs)
{
	return compare_int_keys(lhs->expiration().microSecondsSinceEpoch(), rhs->expiration().microSecondsSinceEpoch()) == 1;
}

void min_heap_ctor(min_heap_t* s)
{
	s->event = 0;
	s->size = 0;
	s->capacity = 0;

}

void min_heap_dtor(min_heap_t* s) { if (s->event) free(s->event); }
void min_heap_elem_init(Timer* e) { e->min_heap_idx = -1; };
int min_heap_push(min_heap_t* s, Timer* ele)
{
	if (min_heap_reserve(s, s->size + 1))
		return -1;
	min_heap_shift_up_(s, s->size++, ele);
	return 0;
}
Timer* min_heap_pop(min_heap_t* s)
{
	if (s->size)
	{
		Timer* e = *s->event;
		min_heap_shift_down_(s, 0u, s->event[--(s->size)]);
		return e;
	}
	return 0;
}

int min_heap_erase(min_heap_t*s, Timer* e)
{
	if (-1 != e->min_heap_idx)
	{
		Timer * last = s->event[--(s->size)];
		unsigned parent = GET_PARENT(e->min_heap_idx);
		if (e->min_heap_idx > 0 && min_heap_elem_greater(s->event[parent], last))
			min_heap_shift_up_(s, e->min_heap_idx, last);
		else
			min_heap_shift_down_(s, e->min_heap_idx, last);
		e->min_heap_idx = -1;
		return 0;
	}
	return -1;
}

int	min_heap_reserve(min_heap_t* s, unsigned n)
{
	if (s->capacity < n)
	{
		Timer ** p;
		unsigned capacity = s->capacity ? s->capacity * 2 : 8;
		if (capacity < n)
			capacity = n;
		if (!(p = (Timer**)realloc(s->event, capacity * sizeof(p))))
			return -1;
		s->event = p;
		s->capacity = capacity;
	}
	return 0;
}

void min_heap_shift_up_(min_heap_t* s, unsigned hole_index,Timer* e)
{
	//unsigned parent = (hole_index - 1) / 2;
	unsigned parent = GET_PARENT(hole_index);
	while (hole_index && min_heap_elem_greater(s->event[parent], e))
	{
		(s->event[hole_index] = s->event[parent])->min_heap_idx = hole_index;
		hole_index = parent;
		//parent = (hole_index - 1) / 2;
		parent = GET_PARENT(hole_index);
	}
	(s->event[hole_index] = e)->min_heap_idx = hole_index;
}
void min_heap_shift_down_(min_heap_t* s, unsigned hole_index, Timer* e)
{
	//unsigned min_child = 2 * (hole_index + 1);
	unsigned min_child = GET_RIGHT_CHILD(hole_index);
	while (min_child <= s->size)
	{
		//find the min child
		if (min_child == s->size || min_heap_elem_greater(s->event[min_child], s->event[min_child - 1]))
		{
			min_child -= 1;
		}
		if (!(min_heap_elem_greater(e, s->event[min_child])))
			break;
		(s->event[hole_index] = s->event[min_child])->min_heap_idx = hole_index;
		hole_index = min_child;
		//min_child = 2 * (hole_index + 1);
		min_child = GET_RIGHT_CHILD(hole_index);
	}
	(s->event[hole_index] = e)->min_heap_idx = hole_index;
}


int min_heap_elt_is_top(const Timer *e) { return e->min_heap_idx == 0; }
int min_heap_empty(min_heap_t* s) { return 0u == s->size; };
unsigned min_heap_size(min_heap_t* s) { return s->size; }
Timer* min_heap_top(min_heap_t* s) { return s->size ? *s->event : 0; }


#endif //CALM_NET_TIMEMINHEAP_H_

