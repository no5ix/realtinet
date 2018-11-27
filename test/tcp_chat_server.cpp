#include "tcp_codec.h"

#include <base/Logging.h>
#include <base/Mutex.h>
#include <net/EventLoop.h>
#include <net/TcpServer.h>

#include <set>
//#include <stdio.h>
//#include <unistd.h>

#include <set>
//#include <stdio.h>
//#include <unistd.h>


using namespace muduo;
using namespace muduo::net;

class TcpChatServer : noncopyable
{
public:
	TcpChatServer(EventLoop* loop,
		const InetAddress& listenAddr)
		: server_(loop, listenAddr, "TcpChatServer"),
		codec_(std::bind(&TcpChatServer::onStringMessage, this, _1, _2, _3)),
		connections_(new ConnectionList)
	{
		server_.setConnectionCallback(
			std::bind(&TcpChatServer::onConnection, this, _1));
		server_.setMessageCallback(
			std::bind(&TcpLengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
	}

	void setThreadNum(int numThreads)
	{
		server_.setThreadNum(numThreads);
	}

	void start()
	{
		server_.start();
	}

private:
	void onConnection(const TcpConnectionPtr& conn)
	{
		LOG_INFO << conn->localAddress().toIpPort() << " -> "
			<< conn->peerAddress().toIpPort() << " is "
			<< (conn->connected() ? "UP" : "DOWN");

		MutexLockGuard lock(mutex_);
		// std::unique_lock<std::mutex> lck(mutex_);
		if (!connections_.unique())
		{
			connections_.reset(new ConnectionList(*connections_));
		}

		assert(connections_.unique());

		if (conn->connected())
		{
			connections_->insert(conn);
		}
		else
		{
			connections_->erase(conn);
		}
	}

	typedef std::set<TcpConnectionPtr> ConnectionList;
	typedef std::shared_ptr<ConnectionList> ConnectionListPtr;

	void onStringMessage(const TcpConnectionPtr&,
		const string& message,
		Timestamp)
	{
		//LOG_INFO << "into TcpChatServer::onStringMessage";
		ConnectionListPtr connections = getConnectionList();
		for (ConnectionList::iterator it = connections->begin();
			it != connections->end();
			++it)
		{
			//LOG_INFO << "into TcpChatServer::onStringMessage codec_.send(get_pointer(*it), message);";
			codec_.send(get_pointer(*it), message);
		}
	}

	ConnectionListPtr getConnectionList()
	{
		MutexLockGuard lock(mutex_);
		// std::unique_lock<std::mutex> lck(mutex_);
		return connections_;
	}

	TcpServer server_;
	TcpLengthHeaderCodec codec_;
	MutexLock mutex_;
	// std::mutex mutex_;
	ConnectionListPtr connections_;
};

#ifndef _WIN32
int main(int argc, char* argv[])
#else
//int main(int argc, char* argv[])
int TcpChatServer_main(int argc, char* argv[])
#endif
{
	//LOG_INFO << "pid = " << getpid();
	// LOG_INFO << "ThreadId = " << getCurrentThreadId();
	if (argc > 1)
	{
		EventLoop loop;
		uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
		InetAddress serverAddr(port);
		TcpChatServer server(&loop, serverAddr);
		if (argc > 2)
		{
			server.setThreadNum(atoi(argv[2]));
		}
		server.start();
		loop.loop();
	}
	else
	{
		EventLoop loop;
		//uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
		InetAddress serverAddr(2000);
		TcpChatServer server(&loop, serverAddr);
		//if (argc > 2)
		{
			//server.setThreadNum(1);
		}
		server.start();
		loop.loop();
	}
	//else
	//{
	//	printf("Usage: %s port [thread_num]\n", argv[0]);
	//}
	return 0;
}

