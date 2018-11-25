//////////////////////////////////////////////////////////////////////////
//
// Copyright(C)	2016, gjm_kevin.  All rights reserved.
// Author	:	gjm_kevin@163.com
//
//////////////////////////////////////////////////////////////////////////
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"
#include <utility>  
using namespace muduo;
using namespace muduo::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const string& nameArg)
	: baseLoop_(baseLoop),
	name_(nameArg),
	started_(false),
	numThreads_(0),
	next_(0)
{}

//EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop)
//	:baseLoop_(baseLoop),
//	started_(false),
//	numThreads_(0),
//	next_(0)
//{}
EventLoopThreadPool::~EventLoopThreadPool()
{
	// Don't delete loop, it's stack variable
}
void EventLoopThreadPool::start(const ThreadInitCallback& cb /* = ThreadInitCallback() */)
{
	assert(!started_);
	baseLoop_->assertInLoopThread();
	started_ = true;

	for (int i=0;i<numThreads_;++i)
	{
		std::unique_ptr<EventLoopThread> foo(new EventLoopThread(cb)) ;
		loops_.push_back(foo->startLoop());
		threads_.push_back(std::move(foo));
	}
	if (numThreads_ == 0 && cb)
	{
		cb(baseLoop_);
	}
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
	baseLoop_->assertInLoopThread();
	assert(started_);
	EventLoop* loop = baseLoop_;
	if (!loops_.empty())
	{
		//round-robin
		loop = loops_[next_];
		++next_;
		if (implicit_cast<size_t>(next_) >= loops_.size())
		{
			next_ = 0;
		}
	}
	return loop;
}
EventLoop* EventLoopThreadPool::getLoopForHash(size_t hashCode)
{
	baseLoop_->assertInLoopThread();
	EventLoop* loop = baseLoop_;

	if (!loops_.empty())
	{
		loop = loops_[hashCode % loops_.size()];
	}
	return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
	baseLoop_->assertInLoopThread();
	assert(started_);
	if (loops_.empty())
	{
		return std::vector<EventLoop*>(1, baseLoop_);
	}
	else
	{
		return loops_;
	}
}
