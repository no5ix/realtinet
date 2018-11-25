//////////////////////////////////////////////////////////////////////////
//
// Copyright(C)	2016, gjm_kevin.  All rights reserved.
// Author	:	gjm_kevin@163.com
//
//////////////////////////////////////////////////////////////////////////
#ifndef CALM_NET_TIMERQUEUE_H_
#define CALM_NET_TIMERQUEUE_H_

#include "TimeMinHeap.h"
#include "uncopyable.h"
#include "timestamp.h"
#include "Callbacks.h"

#include <set>

namespace muduo
{
	namespace net
	{
		class EventLoop;
		class Timer;
		class TimerId;

		class TimerQueue:muduo::uncopyable
		{
		public:
			TimerQueue(EventLoop* loop);
			~TimerQueue();
			
			TimerId addTimer(const TimerCallback& cb,
				Timestamp when,
				double interval);

			void cancel(TimerId timerId);

			void expiredProcess(Timestamp now);
			int earliestExpiredTime(Timestamp now);
		private:
			typedef std::set<Timer*> TimerSet;

			void addTimerInLoop(Timer* timer);
			void cancelInLoop(TimerId timerId);

			

			/*	void reset(const std::vector<Entry>& expired, Timestamp now);
				bool insert(Timer* timer);*/

			EventLoop* loop_;
			min_heap_t timeMinHeap_;
			TimerSet timerSet_;
			bool callingExpiredTimers_;
		};// end class TimerQueue
	}//end namespace net
}//end namespace muduo
#endif //CALM_NET_TIMERQUEUE_H_