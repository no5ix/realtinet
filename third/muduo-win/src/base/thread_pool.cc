#include <thread_pool.h>


#include <exception>
#include <algorithm>  //for_each
#include <assert.h>

using namespace muduo;

ThreadPool::ThreadPool(const string& nameArg /* = string("ThreadPool") */)
	:mutex_(),
	notEmpty_(),
	notFull_(),
	maxQueueSize_(0),
	name_(nameArg),
	running_(false)
{
}

ThreadPool::~ThreadPool()
{
	if (running_)
	{
		stop();
	}
}
void ThreadPool::start(int numThreads)
{
	assert(thread_.empty());
	running_ = true;
	thread_.reserve(numThreads);
	for (int i = 0; i < numThreads; ++i)
	{
		// or use std::move to move an exits object
		thread_.push_back(std::unique_ptr<std::thread>(new std::thread(std::bind(&ThreadPool::_runInThread,this))));
	}
	if (numThreads == 0 && threadInitCallback_)
	{
		threadInitCallback_();
	}
}
void ThreadPool::stop()
{
	{
		std::unique_lock<std::mutex> lck(mutex_);
		running_ = false;
		notEmpty_.notify_all();
	}
	std::vector<std::unique_ptr<std::thread> >::iterator iter;
	for (iter = thread_.begin(); iter != thread_.end();++iter)
	{
		(*iter)->join();
	}
}
void ThreadPool::run(const Task& task)
{
	if (thread_.empty())
	{
		task();
	}
	else
	{
		std::unique_lock<std::mutex> lck(mutex_);
		while (_isFull())
		{
			notFull_.wait(lck);
		}
		queue_.push_back(task);
		notEmpty_.notify_one();
	}
}
ThreadPool::Task ThreadPool::_take()
{
	std::unique_lock<std::mutex> lck(mutex_);
	while (queue_.empty() && running_)
	{
		notEmpty_.wait(lck);
	}
	Task task;
	if (!queue_.empty())
	{
		task = queue_.front();
		queue_.pop_front();
		if (maxQueueSize_ > 0)
			notFull_.notify_one();
	}
	return task;
}
void ThreadPool::_runInThread()
{
	try
	{
		if (threadInitCallback_)
		{
			threadInitCallback_();
		}
		while (running_)
		{
			Task task(_take());
			if (task)
				task();
		}
	}
	catch (const std::exception ex)
	{
		abort();
	}
	catch (...)
	{
		abort();
	}

}

size_t ThreadPool::queueSize() const
{
	std::unique_lock<std::mutex> lck(mutex_);
	return queue_.size();
}
bool ThreadPool::_isFull() const
{
	return maxQueueSize_ > 0 && queue_.size() > maxQueueSize_;
}