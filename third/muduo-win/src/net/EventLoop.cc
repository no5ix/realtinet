// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

//#pragma comment( lib, "ws2_32.lib"  )  // for select 
//#pragma comment(lib,"ws2_32")
//#include <WinSock2.h>
//#include "EventLoop.h"
//
//#include "logging.h"
//#include <assert.h>
//#include <WinBase.h>
//#include <direct.h>
//
//#include <WS2tcpip.h>
//#include <Windows.h>
#include <EventLoop.h>
#include <logging.h>
#include <Channel.h>
#include <Poller.h>
#include <TimerQueue.h>

#include <signal.h>

using namespace muduo;
using namespace muduo::net;



//int initWinSock()
//{
//	WSADATA wsaData;
//	WORD wVersionRequested = MAKEWORD(1, 1);
//	if (0 == WSAStartup(wVersionRequested, &wsaData))
//	{
//		LOG_INFO << "RUN-MSG:Success to connect to the lib of socket!";
//		return 0;
//	}
//	else
//	{
//		LOG_INFO << "ERR-MSG:Failed to connect to the lib of socket!";
//		return -1;
//	}
//	//wVersionRequested是我们要求使用的WinSock的版本。
//}

namespace
{
	thread_local EventLoop* t_loopInThisThread = 0;
	const int kPollTimeMs = 10000;
	class IgnoreSigPipe
	{
	public:
		IgnoreSigPipe()
		{
			
#ifdef _MSC_VER
			//windows use WSAStartUp to handle these things. Or you can think that windows ignore these things
#else
			::signal(SIGABRT, SIG_IGN);
#endif
		}
		
	};// end class IgnoreSigPipe
	IgnoreSigPipe initObj;
}// end anonymous namespace

EventLoop::EventLoop()
	: looping_(false),
	quit_(false),
	eventHandling_(false),
	callingPendingFunctors_(false),
	threadId_(muduo::getCurrentThreadId()),
	poller_(Poller::newDefaultPoller(this)),
	wakeupFd_(sockets::pipe()),
	wakeupChannel_ (new Channel(this, wakeupFd_.pipe_read)),
	currentActiveChannel_(NULL),
	timerQueue_(new TimerQueue(this))
{
	LOG_DEBUG << "EventLoop created " << this << " in thread " << threadId_;
	if (t_loopInThisThread)
	{
		LOG_FATAL << "Another EventLoop " << t_loopInThisThread
			<< " exists in this thread " << threadId_;
	}
	else
	{
		t_loopInThisThread = this;
	}
	wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
	wakeupChannel_->enableReading();

}

EventLoop::~EventLoop()
{
	LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_ << "  destructs in thread " << getCurrentThreadId();
	wakeupChannel_->disableAll();
	wakeupChannel_->remove();
	closesocket(wakeupFd_.pipe_read);
	closesocket(wakeupFd_.pipe_write);
	t_loopInThisThread = NULL;
}

void EventLoop::loop()
{
	assert(!looping_);
	assertInLoopThread();
	looping_ = true;

	quit_ = false;
	LOG_TRACE << "EventLoop " << this << " start looping";
	while (!quit_)	
	{
		activeChannels_.clear();
		pollReturnTime_ = poller_->poll(timerQueue_->earliestExpiredTime(Timestamp::now()), &activeChannels_);
		if (Logger::logLevel() <= Logger::TRACE)
		{
			printActiveChannels();
		}
		eventHandling_ = true;
		timerQueue_->expiredProcess(pollReturnTime_);
		for (ChannelList::iterator it = activeChannels_.begin();
			it != activeChannels_.end(); ++it)
		{
			currentActiveChannel_ = *it;
			currentActiveChannel_->handleEvent(pollReturnTime_);
		}
		currentActiveChannel_ = NULL;
		eventHandling_ = false;
		doPendingFunctors();
	}
	LOG_TRACE << "EventLoop " << this << " stop looping";
	looping_ = false;
	//timeval timeout;
	//timeout.tv_sec = 10;	//wait_timeout 1 second
	//timeout.tv_usec = 0;

	//fd_set read_set, write_set, excep_set;
	//FD_ZERO(&read_set);
	//FD_ZERO(&write_set);
	//FD_ZERO(&excep_set);

	//int nfds = 0;
	//do {
	//	nfds = select(0, NULL, NULL, NULL, &timeout);
	//} while (nfds < 0 && errno == EINTR);

	
}

EventLoop * EventLoop::getEventLoopOfCurrentThread()
{
	return t_loopInThisThread;
}

void EventLoop::abortNotInLoopThread()
{
	LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
		<< " was created in threadId_ = " << threadId_
		<< ", current thread id = " << muduo::getCurrentThreadId();
}

void EventLoop::quit()
{
	quit_ = true;
	if (!isInLoopThread())
	{
		wakeup();
	}
}

TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback& cb)
{
	return timerQueue_->addTimer(cb, time, 0.0);
}
TimerId EventLoop::runAfter(double delay, const TimerCallback& cb)
{
	Timestamp time(addTime(Timestamp::now(), delay));
	return runAt(time, cb);
}
TimerId EventLoop::runEvery(double interval, const TimerCallback& cb)
{
	Timestamp time(addTime(Timestamp::now(), interval));
	return timerQueue_->addTimer(cb, time, interval);
}

void EventLoop::cancel(TimerId timerId)
{
	timerQueue_->cancel(timerId);
}

void EventLoop::runInLoop(const Functor & cb)
{
	if (isInLoopThread())
	{
		cb();
	}
	else
	{
		queueInLoop(cb);
	}
}
void EventLoop::queueInLoop(const Functor&cb)
{
	{
		std::unique_lock<std::mutex> lck(mutex_);
		pendingFunctors_.push_back(cb);
	}
	if (!isInLoopThread() || callingPendingFunctors_)
	{
		//callingPendingFunctors_ make the next poll return immediately
		wakeup();
	}
}


void EventLoop::updateChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	poller_->updateChannel(channel);
}
void EventLoop::removeChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	if (eventHandling_)
	{
		assert(currentActiveChannel_ == channel || std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
	}
	poller_->removeChannel(channel);
}
bool EventLoop::hasChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	return poller_->hasChannel(channel);
}
void EventLoop::wakeup()
{
	uint64_t one = 1;
	ssize_t n = sockets::write(wakeupFd_.pipe_write, &one, sizeof(one));
	if (n != sizeof(one))
	{
		LOG_ERROR << "EventLoop::wakeup() writes" << n << " bytes instead of 8(int64_t)";
	}
}
void EventLoop::handleRead()
{
	uint64_t one = 1;
	ssize_t n = sockets::read(wakeupFd_.pipe_read, &one, sizeof one);
	if (n != sizeof(one))
	{
		LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8(int64_t)";
	}
}

void EventLoop::doPendingFunctors()
{
	std::vector<Functor> functors;
	callingPendingFunctors_ = true;
	{
		std::unique_lock<std::mutex> lck(mutex_);
		functors.swap(pendingFunctors_);
	}
	for (size_t i = 0; i < functors.size(); ++i)
	{
		functors[i]();
	}
	callingPendingFunctors_ = false;
}
void EventLoop::printActiveChannels()const
{
	for (ChannelList::const_iterator it = activeChannels_.begin();
		it != activeChannels_.end(); ++it)
	{
		const Channel* ch = *it;
		LOG_TRACE << "{" << ch->reventsToString() << "} ";
	}
}