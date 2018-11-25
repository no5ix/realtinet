//////////////////////////////////////////////////////////////////////////
//
// Copyright(C)	2016, gjm_kevin.  All rights reserved.
// Author	:	gjm_kevin@163.com
//
//////////////////////////////////////////////////////////////////////////
#include "EventLoop.h"
#include "TimerQueue.h"
#include "logging.h"
#include "Timer.h"
#include "TimerId.h"
#include <functional>

using namespace muduo;
using namespace muduo::net;

namespace
{
	const int defaultOuttime = 100;
	const int defaultTimeout = 10000;
}


TimerQueue::TimerQueue(EventLoop* loop)
	:loop_(loop),
	timerSet_()
{
	min_heap_ctor(&timeMinHeap_);
}
TimerQueue::~TimerQueue()
{
	for (TimerSet::iterator it = timerSet_.begin();
		it != timerSet_.end(); ++it)
	{
		delete *it;
	}
	min_heap_dtor(&timeMinHeap_);
}
TimerId TimerQueue::addTimer(const TimerCallback& cb, Timestamp when, double interval)
{
	Timer* timer = new Timer(cb, when, interval);
	loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
	return TimerId(timer, timer->sequence());
 }
void TimerQueue::cancel(TimerId timerId)
{
	loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerId));
}
void TimerQueue::addTimerInLoop(Timer* timer)
{
	loop_->assertInLoopThread();
	Timestamp when = timer->expiration();
	min_heap_push(&timeMinHeap_, timer);
	timerSet_.insert(timer);
}
void TimerQueue::cancelInLoop(TimerId timerId)
{
	loop_->assertInLoopThread();
	//FIXME:May run timeout func after cancel. Do not delete from PendingFunctors.
	//Memory not delete here, because we may use the pointer in PendingFunctors.
	min_heap_erase(&timeMinHeap_, timerId.timer_);
	//TimerSet::iterator it = timerSet_.find(timerId.timer_);
	//if (it != timerSet_.end())
	//{
	//	timerSet_.erase(timerId.timer_);
	//}
	
}

void TimerQueue::expiredProcess(Timestamp now)
{
	while (1)
	{
		Timer* timer = min_heap_top(&timeMinHeap_);
		if(!timer)
			break;
		if (timer->expiration().microSecondsSinceEpoch() > now.microSecondsSinceEpoch())
			break;
		
		loop_->runInLoop(std::bind(&Timer::run,timer));
		min_heap_pop(&timeMinHeap_);
		if (timer->repeat())
		{
			timer->restart(now);
			min_heap_push(&timeMinHeap_, timer);
		}
	}
}

int TimerQueue::earliestExpiredTime(Timestamp now)
{
	Timer* timer = min_heap_top(&timeMinHeap_);
	// No timer 
	if (!timer)
		return defaultTimeout;
	if (timer->expiration().microSecondsSinceEpoch() > now.microSecondsSinceEpoch())
	{
		return timeDifferenceMs( timer->expiration(),now);
	}
	// Already has timeout timer
	return defaultOuttime;
}


