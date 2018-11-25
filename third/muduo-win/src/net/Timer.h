//////////////////////////////////////////////////////////////////////////
//
// Copyright(C)	2016, gjm_kevin.  All rights reserved.
// Author	:	gjm_kevin@163.com
//
//////////////////////////////////////////////////////////////////////////
#ifndef CALM_NET_TIMER_H_
#define CALM_NET_TIMER_H_
#include "uncopyable.h"
#include "timestamp.h"
#include "Callbacks.h"

#include <atomic>

namespace muduo
{
	namespace net
	{
		class Timer :muduo::uncopyable
		{
		public:
			Timer(const TimerCallback& cb, Timestamp when, double interval)
				:callback_(cb),
				expiration_(when),
				interval_(interval),
				repeat_(interval > 0.0),
				sequence_(s_numCreated_++)
			{}
			void run() const 
			{
				if(callback_)callback_();
			}
			void restart(Timestamp now);

			Timestamp expiration() const { return expiration_; }
			bool repeat() const { return repeat_; }
			int64_t sequence() const { return sequence_; }
			static int64_t numCreated() { return s_numCreated_; }
			int min_heap_idx;
		private:
			const TimerCallback callback_;
			Timestamp expiration_;
			const double interval_;
			const bool repeat_;
			const int64_t sequence_;

			static std::atomic_int64_t s_numCreated_;
		};// end class Timer
	}// end namespace net
}//end namespace muduo
#endif //CALM_NET_TIMER_H_