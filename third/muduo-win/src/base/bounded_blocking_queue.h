#ifndef CALM_BASE_BOUNDEDBLOCKINGQUEUE_H_
#define CALM_BASE_BOUNDEDBLOCKINGQUEUE_H_

#include <uncopyable.h>

#include <deque>
#include <mutex>
#include <condition_variable>

namespace muduo
{
	template<typename T>
	class BoundedBlockingQueue :muduo::uncopyable
	{
	public:
		BoundedBlockingQueue(int maxQueueSize)
			:mutex_(),
			 notFull_(),
			 notEmpty_(),
			 maxQueueSize_(0)

		{}
		void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
		void put(T& x)
		{
			if (maxQueueSize_ == 0)
				return;
			std::unique_lock<std::mutex> lck(mutex_);
			while (isFull())
			{
				notFull_.wait(lck);
			}
			queue_.push_back(x);
			notEmpty_.notify_one();
		}
		T take()
		{
			if (maxQueueSize_ == 0)
				return;
			std::unique_lock<std::mutex> lck(mutex_);
			while (empty())
			{
				notEmpty_.wait(lck);
			}
			T front(queue_.front());
			queue_.pop_front();
			notFull_.notify_one();
			return front;
		}
		bool isFull() const
		{
			return maxQueueSize_ > 0 && queue_.size() > maxQueueSize_;
		}
		bool empty() const
		{
			std::unique_lock<std::mutex> lck(mutex_);
			return queue_.empty();
		}
		bool full() const 
		{
			std::unique_lock<std::mutex> lck(mutex_);
			return queue_.size() == maxQueueSize_;
		}
		size_t size() const
		{
			std::unique_lock<std::mutex> lck(mutex_);
			return queue_.size();
		}
		size_t capacity()const { return maxQueueSize_; }


	private:
		std::mutex mutex_;
		std::condition_variable notFull_;
		std::condition_variable notEmpty_;
		size_t maxQueueSize_;
		std::deque<T> queue_;
	};
} //end namespace muduo

#endif //CALM_BASE_BOUNDEDBLOCKINGQUEUE_H_