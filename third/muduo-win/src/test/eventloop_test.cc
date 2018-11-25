#include "EventLoop.h"
#include "logging.h"

#include <thread>
#include <iostream>

using namespace muduo;
using namespace muduo::net;

EventLoop *g_loop;
void net_func()
{
	LOG_INFO << "func:pid = " << getCurrentThreadId();
}
void threadFunc()
{
	Sleep(2000);
	LOG_INFO << "threadFunc:pid = " << getCurrentThreadId();
	g_loop->runInLoop(net_func);
}

//int main()
//{
//	WSADATA  Ws;
//	//Init Windows Socket
//	if (WSAStartup(MAKEWORD(2, 2), &Ws) != 0)
//	{
//		LOG_FATAL<< "Init Windows Socket Failed::" << GetLastError();
//		return -1;
//	}
//	EventLoop loop;
//	g_loop = &loop;
//	std::thread th(threadFunc);
//	loop.loop();
//	//std::cin.get();
//	return 0;
//}