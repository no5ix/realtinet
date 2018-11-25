//////////////////////////////////////////////////////////////////////////
//
// Copyright(C)	2016, gjm_kevin.  All rights reserved.
// Author	:	gjm_kevin@163.com
//
//////////////////////////////////////////////////////////////////////////
#ifndef CALM_NET_EVENTLOOPTHREADPOOL_H_
#define CALM_NET_EVENTLOOPTHREADPOOL_H_
#include "types.h"
#include "uncopyable.h"
#include <functional>
#include <vector>
#include <memory>

namespace muduo
{
	namespace net
	{
		class EventLoop;
		class EventLoopThread;
		class EventLoopThreadPool :muduo::uncopyable
		{
		public:
			typedef std::function<void(EventLoop*)> ThreadInitCallback;

			EventLoopThreadPool(EventLoop* baseLoop, const string& nameArg);
			//EventLoopThreadPool(EventLoop* baseLoop);
			~EventLoopThreadPool();
			void setThreadNum(int numThreads) { numThreads_ = numThreads; }
			void start(const ThreadInitCallback& cb = ThreadInitCallback());

			//valid after calling start()
			// round-robin
			EventLoop* getNextLoop();
			/// with the same hash code, it will always return the same EventLoop
			EventLoop* getLoopForHash(size_t hashCode);

			std::vector<EventLoop*> getAllLoops();
			
			bool started() const { return started_; }

			const string& name() const
			{ return name_; }
			
		private:
			EventLoop* baseLoop_;
			string name_;
			bool started_;
			int numThreads_;
			int next_;
			std::vector<std::unique_ptr<EventLoopThread> > threads_;
			// just use the pointer, dtor use threads_ unique_ptr
			std::vector<EventLoop*> loops_;

		};// end class EventLoopThreadPool
	}// end namespace net
}// end namespace muduo

#endif //CALM_NET_EVENTLOOPTHREADPOOL_H_