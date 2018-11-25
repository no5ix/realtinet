//////////////////////////////////////////////////////////////////////////
//
// Copyright(C)	2016, gjm_kevin.  All rights reserved.
// Author	:	gjm_kevin@163.com
//
//////////////////////////////////////////////////////////////////////////
#ifndef CALM_NET_EVENTLOOPTHREAD_H_
#define CALM_NET_EVENTLOOPTHREAD_H_
#include "uncopyable.h"
#include "types.h"
#include <condition_variable>
#include <mutex>
#include <thread>
#include <functional>

namespace muduo
{
	namespace net
	{
		class EventLoop;
		class EventLoopThread :muduo::uncopyable
		{
		public:
			typedef std::function<void(EventLoop*)> ThreadInitCallback;
			EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback());
			~EventLoopThread();

			EventLoop* startLoop();

		private:
			void threadFunc();

			EventLoop* loop_;
			bool exiting_;
			// if thread_ not started, dtor will abort
			bool threadStarting_;
			std::thread thread_;
			std::mutex mutex_;
			std::condition_variable cond_;
			ThreadInitCallback callback_;
			

		};// end class EventLoopThread
	}// end namespace net
}// end namespace muduo
#endif //CALM_NET_EVENTLOOPTHREAD_H_
