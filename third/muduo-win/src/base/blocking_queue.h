#ifndef CALM_BASE_BLOCKINGQUEUE_H_
#define CALM_BASE_BLOCKINGQUEUE_H_

#include <uncopyable.h>

#include <mutex>
#include <condition_variable>
#include <deque>

namespace muduo
{
	template<typename T>
	class BlockingQueue :muduo::uncopyable
	{
	public:
		BlockingQueue()
			:mutex_(),
			condition_(),
			queue_()
		{}
		void put(const T& x )
		{
			std::unique_lock<std::mutex> lck(mutex_);
			queue_.push_back(x);
			condition_.notify_one();
		}
		T take()
		{
			std::unique_lock<std::mutex> lck(mutex_);
			while(queue_.empty())
				condition_.wait(lck);
			T font(queue_.front());
			queue_.pop_front();
			return x;
		}
		size_t size() const
		{}
	private:
		std::mutex mutex_;
		std::condition_variable condition_;
		std::deque<T> queue_;
	};
}// end namespace muduo

#endif  //CALM_BASE_BLOCKINGQUEUE_H_
