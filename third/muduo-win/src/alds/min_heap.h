//////////////////////////////////////////////////////////////////////////
//
// Copyright(C)	2016, gjm_kevin.  All rights reserved.
// Author	:	gjm_kevin@163.com
//
//////////////////////////////////////////////////////////////////////////

#ifndef CALM_ALDS_MINHEAP_H_
#define CALM_ALDS_MINHEAP_H_

///General min or max heap, change the compare_func to make it work
///Store user date in element.value


#include <malloc.h>

#define GET_LEFT_CHILD(i)   ((i*2)+1)
#define GET_RIGHT_CHILD(i)  ((i+1)*2)
#define GET_PARENT(i) ((i-1)/2)

struct element
{
	void * value;
	int min_heap_idx;
};

typedef struct min_heap
{
	struct element** value;
	//void ** value;
	unsigned size;   // num of heap
	unsigned capacity; 
	int(*compare_func)(void*, void*);
} min_heap_t;

typedef int(*comp_func)(void*, void*);

static inline void	min_heap_ctor(min_heap_t* s, comp_func func);
static inline void	min_heap_dtor(min_heap_t* s);
static inline void	min_heap_elem_init(struct element* e);
static inline int	min_heap_push(min_heap_t* s, struct element* ele);
static inline struct element* min_heap_pop(min_heap_t* s);
static inline int min_heap_erase(min_heap_t*s, struct element* e);
static inline int	min_heap_reserve(min_heap_t* s, unsigned n);
static inline void	min_heap_shift_up_(min_heap_t* s, unsigned hole_index, struct element* e);
static inline void	min_heap_shift_down_(min_heap_t* s, unsigned hole_index, struct element* e);

static inline int	     min_heap_elt_is_top(const struct element *e);
static inline int	     min_heap_empty(min_heap_t* s);
static inline unsigned	     min_heap_size(min_heap_t* s);
static inline struct element*  min_heap_top(min_heap_t* s);

int compare_int_keys(register void* key1, register void* key2) {
	// Cast them as int* and read them in
	register int key1_v = *((int*)key1);
	register int key2_v = *((int*)key2);
	// Perform the comparison
	if (key1_v < key2_v)
		return -1;
	else if (key1_v == key2_v)
		return 0;
	else
		return 1;
}


int min_heap_elem_greater(min_heap_t *s, struct element *lhs, struct element *rhs)
{
	return s->compare_func(lhs->value,rhs->value) == 1;
}

void min_heap_ctor(min_heap_t* s, comp_func func) 
{ 
	s->value = 0;
	s->size = 0;
	s->capacity = 0;
	if (func == NULL)
		s->compare_func = compare_int_keys;
}

void min_heap_dtor(min_heap_t* s) { if (s->value) free(s->value); }
void min_heap_elem_init(struct element* e) { e->min_heap_idx = -1; };
int min_heap_push(min_heap_t* s, struct element* ele)
{
	if (min_heap_reserve(s, s->size + 1))
		return -1;
	min_heap_shift_up_(s, s->size++, ele);
	return 0;
}
struct element* min_heap_pop(min_heap_t* s)
{
	if (s->size)
	{
		struct element* e = *s->value;
		min_heap_shift_down_(s, 0u, s->value[--(s->size)]);
		return e;
	}
	return 0;
}

int min_heap_erase(min_heap_t*s, struct element* e)
{
	if (-1 != e->min_heap_idx)
	{
		struct element * last = s->value[--(s->size)];
		unsigned parent = GET_PARENT(e->min_heap_idx);
		if (e->min_heap_idx > 0 && min_heap_elem_greater(s, s->value[parent], last))
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
		struct element ** p;
		unsigned capacity = s->capacity ? s->capacity * 2 : 8;
		if (capacity < n)
			capacity = n;
		if (!(p = (struct element**)realloc(s->value, capacity * sizeof(p))))
			return -1;
		s->value = p;
		s->capacity = capacity;
	}
	return 0;
}

void min_heap_shift_up_(min_heap_t* s, unsigned hole_index, struct element* e)
{
	//unsigned parent = (hole_index - 1) / 2;
	unsigned parent =GET_PARENT(hole_index);
	while (hole_index && min_heap_elem_greater(s,s->value[parent], e))
	{
		(s->value[hole_index] = s->value[parent])->min_heap_idx = hole_index;
		hole_index = parent;
		//parent = (hole_index - 1) / 2;
		parent = GET_PARENT(hole_index);
	}
	(s->value[hole_index] = e)->min_heap_idx = hole_index;
}
void min_heap_shift_down_(min_heap_t* s, unsigned hole_index, struct element* e)
{
	//unsigned min_child = 2 * (hole_index + 1);
	unsigned min_child = GET_RIGHT_CHILD(hole_index);
	while (min_child <= s->size)
	{
		//find the min child
		if (min_child == s->size || min_heap_elem_greater(s,s->value[min_child], s->value[min_child - 1]))
		{
			min_child -= 1;
		}
		if (!(min_heap_elem_greater(s,e, s->value[min_child])))
			break;
		(s->value[hole_index] = s->value[min_child])->min_heap_idx = hole_index;
		hole_index = min_child;
		//min_child = 2 * (hole_index + 1);
		min_child = GET_RIGHT_CHILD(hole_index);
	}
	(s->value[hole_index] = e)->min_heap_idx = hole_index;
}


int min_heap_elt_is_top(const struct element *e) { return e->min_heap_idx == 0; }
int min_heap_empty(min_heap_t* s) { return 0u == s->size; };
unsigned min_heap_size(min_heap_t* s) { return s->size; }
struct element* min_heap_top(min_heap_t* s) { return s->size ? *s->value : 0; }


#endif //CALM_ALDS_MINHEAP_H_

