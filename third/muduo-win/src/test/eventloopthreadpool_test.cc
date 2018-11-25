//#include "EventLoopThreadPool.h"
//#include "EventLoop.h"
//#include "thread.h"
//#include "logging.h"
//#include <functional>
//
//using namespace muduo;
//using namespace muduo::net;
//
//void print_id(EventLoop* p = NULL)
//{
//	LOG_INFO << "print_id func: threadId = " << getCurrentThreadId() << " loop = " << p;
//}
//void init(EventLoop* p)
//{
//	LOG_INFO << "init func: threadId = " << getCurrentThreadId() << " loop = " << p;
//}
//
//int main_eventloopthreadpool()
//{
//	print_id();
//	EventLoop loop;
//	
//	{
//		LOG_INFO << "single thread : " << &loop;
//		EventLoopThreadPool model(&loop);
//		model.setThreadNum(0);
//		model.start(init);
//		assert(model.getNextLoop() == &loop);
//		assert(model.getNextLoop() == &loop);
//		assert(model.getNextLoop() == &loop);
//		std::this_thread::sleep_for(std::chrono::seconds(5));
//	}
//
//	{
//		LOG_INFO<<"Another thread";
//		EventLoopThreadPool model(&loop);
//		model.setThreadNum(1);
//		model.start(init);
//		EventLoop* nextLoop = model.getNextLoop();
//		nextLoop->queueInLoop(std::bind(print_id, nextLoop));
//		assert(nextLoop != &loop);
//		assert(nextLoop == model.getNextLoop());
//		assert(nextLoop == model.getNextLoop());
//		std::this_thread::sleep_for(std::chrono::seconds(3));
//	}
//
//	{
//		LOG_INFO << "Three threads";
//		EventLoopThreadPool model(&loop);
//		model.setThreadNum(3);
//		model.start(init);
//		EventLoop* nextLoop = model.getNextLoop();
//		nextLoop->runInLoop(std::bind(print_id, nextLoop));
//		assert(nextLoop != &loop);
//		assert(nextLoop != model.getNextLoop());
//		assert(nextLoop != model.getNextLoop());
//		assert(nextLoop == model.getNextLoop());
//	}
//
//	//loop.loop();
//	return 0;
//}