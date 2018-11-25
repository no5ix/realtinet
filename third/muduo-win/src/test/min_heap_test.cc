#include <iostream>
#include "min_heap.h"

int main_min_heap()
{
	struct element *inserted[1024];
	struct min_heap heap;
	min_heap_ctor(&heap,NULL);
	for (int i = 1023; i >= 0 ; --i) {
		inserted[i] = (struct element *)malloc(sizeof(struct element));
		inserted[i]->value = malloc(sizeof(int));
		*(int *)(inserted[i]->value) = i;
		min_heap_push(&heap, inserted[i]);
	}
	for (int i = 0; i < 512; ++i) {
		min_heap_erase(&heap, inserted[i]);
	}
	std::cout << "size = " << heap.size << std::endl;
	while (true)
	{
		struct element* p = (struct element *)min_heap_pop(&heap);
		if(!p)
			break;
		printf("%d,", *(int *)(p->value));
	}
	for (int i = 0; i < 1024; ++i)
	{
		free(inserted[i]->value);
		free(inserted[i]);
	}
	min_heap_dtor(&heap);
	std::cin.get();
	return 0;
}