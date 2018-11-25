#include "TcpServer.h"
#include "logging.h"
#include <functional>

using namespace muduo;
using namespace muduo::net;

class EchoServer
{
public:
	EchoServer(EventLoop* loop,InetAddress& listenAddr)
		:loop_(loop),
		server_(loop,listenAddr,"EchoServer")
	{
		server_.setConnectionCallback(std::bind(&EchoServer::onConnection,this,std::placeholders::_1));
		server_.setMessageCallback(std::bind(&EchoServer::onMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
	}
	void start()
	{
		server_.start();
	}
private:
	void onConnection(const TcpConnectionPtr& conn)
	{
		LOG_TRACE << conn->peerAddress().toIpPort() << " -> "
			<< conn->localAddress().toIpPort() << " is "
			<< (conn->connected() ? "UP" : "DOWN");
		conn->send("hello muduo ");
	}
	void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
	{
		string msg(buf->retrieveAllAsString());
		LOG_INFO << conn->name() << " recv " << msg.size() << " bytes at " << time.toString();
		LOG_INFO << conn->name() << " recv " << msg;
		//LOG_INFO << "ThreadId = " << getCurrentThreadId();
		if (msg == "exit\n")
		{
			conn->send("bye\n");
			conn->shutdown();
		}
		if (msg == "quit\n")
		{
			loop_->quit();
		}
		conn->send(msg);
	}
	EventLoop* loop_;
	TcpServer server_;
};//end EchoServer

//int main()
int main_echoserver()
{
	LOG_INFO << "ThreadId = " << getCurrentThreadId();
	EventLoop loop;
	InetAddress listenAddr(2000);
	EchoServer server(&loop, listenAddr);
	server.start();

	loop.loop();
	return 0;
}


//#include <net/TcpServer.h>
//
//#include <base/Logging.h>
//#include <base/Thread.h>
//#include <net/EventLoop.h>
//#include <net/InetAddress.h>
//
////#include <muduo/net/TcpServer.h>
////
////#include <muduo/base/Logging.h>
////#include <muduo/base/Thread.h>
////#include <muduo/net/EventLoop.h>
////#include <muduo/net/InetAddress.h>
//
//#include <utility>
//
//#include <stdio.h>
////#include <unistd.h>
//
//using namespace muduo;
//using namespace muduo::net;
//
//int numThreads = 0;
//
//class EchoServer
//{
// public:
//  EchoServer(EventLoop* loop, const InetAddress& listenAddr)
//    : loop_(loop),
//      server_(loop, listenAddr, "EchoServer")
//  {
//    server_.setConnectionCallback(
//        std::bind(&EchoServer::onConnection, this, _1));
//    server_.setMessageCallback(
//        std::bind(&EchoServer::onMessage, this, _1, _2, _3));
//    server_.setThreadNum(numThreads);
//  }
//
//  void start()
//  {
//    server_.start();
//  }
//  // void stop();
//
// private:
//  void onConnection(const TcpConnectionPtr& conn)
//  {
//    LOG_TRACE << conn->peerAddress().toIpPort() << " -> "
//        << conn->localAddress().toIpPort() << " is "
//        << (conn->connected() ? "UP" : "DOWN");
//    LOG_INFO << conn->getTcpInfoString();
//
//    conn->send("hello\n");
//  }
//
//  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
//  {
//    string msg(buf->retrieveAllAsString());
//    LOG_TRACE << conn->name() << " recv " << msg.size() << " bytes at " << time.toString();
//    if (msg == "exit\n")
//    {
//      conn->send("bye\n");
//      conn->shutdown();
//    }
//    if (msg == "quit\n")
//    {
//      loop_->quit();
//    }
//    conn->send(msg);
//  }
//
//  EventLoop* loop_;
//  TcpServer server_;
//};
//
//int main(int argc, char* argv[])
//{
//  LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
//  LOG_INFO << "sizeof TcpConnection = " << sizeof(TcpConnection);
//  if (argc > 1)
//  {
//    numThreads = atoi(argv[1]);
//  }
//  bool ipv6 = argc > 2;
//  EventLoop loop;
//  InetAddress listenAddr(2000, false, ipv6);
//  EchoServer server(&loop, listenAddr);
//
//  server.start();
//
//  loop.loop();
//}
//
