// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

// modify by:Kevin <gjm_kevin@163.com>


// must be include in the first place when use socket opts.
#ifndef CALM_NET_EVENTLOOP_H_
#define CALM_NET_EVENTLOOP_H_

#include "SocketsOps.h"
#include "timestamp.h"
#include "thread.h"
#include "uncopyable.h"
#include "TimerId.h"
#include "Callbacks.h"

#include <memory>
#include <vector>
#include <mutex>


namespace muduo
{
	namespace net
	{
		class Channel;
		class Poller;
		class TimeQueue; 

		class EventLoop : muduo::uncopyable
		{
		public:
			typedef std::function<void()> Functor;
			EventLoop();
			~EventLoop();
			///loops forever
			///Must be call in the same thread as creation of the eventloop object
			void loop();
			///Quits loop
			/// This is not 100% thread safe, if you call through a raw pointer,
			/// better to call through shared_ptr<EventLoop> for 100% safety.
			void quit();
			Timestamp pollReturnTime() const { return pollReturnTime_; }

			/// Runs callback immediately in the loop thread.
			/// It wakes up the loop, and run the cb.
			/// If in the same loop thread, cb is run within the function.
			/// Safe to call from other threads.
			void runInLoop(const Functor& cb);
			/// Queues callback in the loop thread.
			/// Runs after finish pooling.
			/// Safe to call from other threads.
			void queueInLoop(const Functor&cb);

			/// Timers
			/// Runs callback at 'time'.
			TimerId runAt(const Timestamp& time, const TimerCallback& cb);
			/// Runs callback after @c delay seconds.
			TimerId runAfter(double delay, const TimerCallback& cb);
			/// Runs callback every @c interval seconds.
			TimerId runEvery(double interval, const TimerCallback& cb);
			/// Cancels the timer.
			void cancel(TimerId timerId);

			// internal usage
			void wakeup();

			void updateChannel(Channel* channel);
			void removeChannel(Channel* channel);
			bool hasChannel(Channel* channel);

			void assertInLoopThread()
			{
				if (!isInLoopThread())
				{
					abortNotInLoopThread();
				}
			}

			bool isInLoopThread() const {return threadId_ == muduo::getCurrentThreadId(); }

			bool eventHandling() const { return eventHandling_; }
			//ignore the context
			//void setContext()

			static EventLoop* getEventLoopOfCurrentThread();
		private:

			void abortNotInLoopThread();
			void handleRead();
			void doPendingFunctors();
			void printActiveChannels() const;

			typedef std::vector<Channel*> ChannelList;

			bool looping_; // atomic 
			bool quit_;
			bool eventHandling_;
			bool callingPendingFunctors_;

			const uint64_t threadId_;
			Timestamp pollReturnTime_;
			
			std::shared_ptr<Poller> poller_;

			// use windows pipe to notify the loop 
			stPipe wakeupFd_;
			std::shared_ptr<Channel> wakeupChannel_;

			ChannelList activeChannels_;
			Channel* currentActiveChannel_;
			std::shared_ptr<TimerQueue> timerQueue_;

			std::mutex mutex_;
			std::vector<Functor> pendingFunctors_;
		};
	}// end namespace net
}// end namespace muduo

#endif  // MUDUO_NET_EVENTLOOP_H
