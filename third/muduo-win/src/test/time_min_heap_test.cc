//#include <iostream>
//#include "TimeMinHeap.h"
//
//int main()
//{
//	struct element *inserted[1024];
//	struct min_heap heap;
//	min_heap_ctor(&heap);
//	for (int i = 1023; i >= 0; --i) {
//		inserted[i] = (struct element *)malloc(sizeof(struct element));
//		inserted[i]->timeout = i;
//		min_heap_push(&heap, inserted[i]);
//	}
//	for (int i = 0; i < 512; ++i) {
//		min_heap_erase(&heap, inserted[i]);
//	}
//	std::cout << "size = " << heap.size << std::endl;
//	while (true)
//	{
//		struct element* p = (struct element *)min_heap_pop(&heap);
//		if (!p)
//			break;
//		printf("%d,",p->timeout);
//	}
//	for (int i = 0; i < 1024; ++i)
//	{
//		free(inserted[i]);
//	}
//	min_heap_dtor(&heap);
//	std::cin.get();
//	return 0;
//	return 0;
//}