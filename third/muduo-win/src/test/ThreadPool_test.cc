
#include <thread_pool.h>
#include <count_down_latch.h>
#include <iostream>
#include <stringpiece.h>
#include <timestamp.h>
#include <functional>
#include "thread.h"

using namespace muduo;
void func()
{
	for (int i = 0; i < 100000000; ++i) {
	} // ¼ÆÊý.
}

void func1(int i)
{
	std::cout << "threadId = " << getCurrentThreadId() << "; func in thread pool=" << i << std::endl;
}
int main_threadpool()
{
	{
		ThreadPool pool;
		pool.setMaxQueueSize(100000);
		//pool.setThreadInitCallback(func);
		pool.start(10);

		for (int i = 0; i < 10000000; ++i)
		{
			pool.run(std::bind(func1,i));
		}

		//main thread can not exit, because the jobs not complete. use countdownlatch do this 
		CountDownLatch latch(1);
		pool.run(std::bind(&muduo::CountDownLatch::countDown, &latch));
		latch.wait();
		pool.stop();
	}
	std::cin.get();
	return 0;
}