//////////////////////////////////////////////////////////////////////////
//
// Copyright(C)	2016, gjm_kevin.  All rights reserved.
// Author	:	gjm_kevin@163.com
//
//////////////////////////////////////////////////////////////////////////
#include "EventLoop.h"
#include "EventLoopThread.h"

using namespace muduo;
using namespace muduo::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb /* = ThreadInitCallback() */)
	:loop_(NULL),
	exiting_(false),
	threadStarting_(false),
	thread_(),
	mutex_(),
	cond_(),
	callback_(cb)
{}
EventLoopThread::~EventLoopThread()
{
	exiting_ = true;
	if (loop_ != NULL)
	{
		loop_->quit();
	}
	if (threadStarting_)
	{
		thread_.join();
	}
	
	
}
EventLoop* EventLoopThread::startLoop()
{
	thread_ = std::thread(std::bind(&EventLoopThread::threadFunc, this));
	threadStarting_ = true;
	{
		std::unique_lock<std::mutex> lck(mutex_);
		while (loop_ == NULL)		
		{
			cond_.wait(lck);
		}
	}
	return loop_;
}
void EventLoopThread::threadFunc()
{
	EventLoop loop;
	if(callback_)
	{
		callback_(&loop);
	}
	{
		std::unique_lock<std::mutex> lck(mutex_);
		loop_ = &loop;
		cond_.notify_one();
	}
	loop.loop();
	loop_ = NULL;
}
