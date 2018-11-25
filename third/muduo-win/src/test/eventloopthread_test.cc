//////////////////////////////////////////////////////////////////////////
//
// Copyright(C)	2016, gjm_kevin.  All rights reserved.
// Author	:	gjm_kevin@163.com
//
//////////////////////////////////////////////////////////////////////////
#include "EventLoopThread.h"
#include "EventLoop.h"
#include "logging.h"
#include "count_down_latch.h"

using namespace muduo;
using namespace muduo::net;

void print(EventLoop* loop = NULL)
{
	LOG_INFO << "threadId = " << getCurrentThreadId() << " loop = " << loop;
}
void quit(EventLoop* loop)
{
	print(loop);
	loop->quit();
}

int main_eventLoop()
{
	print();
	{
		EventLoopThread th1;
	}
	{
		EventLoopThread th2;
		EventLoop* loop = th2.startLoop();
		loop->runInLoop(std::bind(print, loop));
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
	{
		EventLoopThread th3;
		EventLoop* loop = th3.startLoop();
		loop->runInLoop(std::bind(quit, loop));
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
	return 0;
}
