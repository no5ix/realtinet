//////////////////////////////////////////////////////////////////////////
//
// Copyright(C)	2016, gjm_kevin.  All rights reserved.
// Author	:	gjm_kevin@163.com
//
//////////////////////////////////////////////////////////////////////////
#ifndef CALM_NET_TIMERID_H_
#define CALM_NET_TIMERID_H_
#include "copyable.h"
#include "types.h"
namespace muduo
{
	namespace net
	{
		class Timer;
		class TimerId :muduo::copyable
		{
		public:
			TimerId()
				:timer_(NULL),
				sequence_(0)
			{}
			TimerId(Timer* timer,int64_t seq)
				:timer_(timer),
				sequence_(seq)
			{}
			// default copy-ctor, dtor and assignment are okay

			friend class TimerQueue;
		private:
			Timer* timer_;
			int64_t sequence_;
		};
	}//end namespace net
}// end namespace muduo
#endif //CALM_NET_TIMERID_H_