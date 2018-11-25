#ifndef CALM_BASE_COUNTDOWNLATCH_H_
#define CALM_BASE_COUNTDOWNLATCH_H_

#include "uncopyable.h"
#include <types.h>

#include <mutex>
#include <condition_variable>
namespace muduo
{
	class CountDownLatch :muduo::uncopyable
	{
	public:
		explicit CountDownLatch(int count);
		void wait();
		void countDown();
		int getCount() const;
	private:
		mutable std::mutex mutex_;
		std::condition_variable condition_;
		int count_;

	};//end class CountDownLatch
}//end namespace muduo

#endif  //CALM_BASE_COUNTDOWNLATCH_H_