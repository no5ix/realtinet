#include "udp_codec.h"

#include <base/Logging.h>
//#include <muduo/base/Logging.h>
//#include <muduo/base/Mutex.h>
#include <net/EventLoopThread.h>
//#include <net/TcpClient.h>
#include <core/UdpClient.h>

#include <iostream>
//#include <stdio.h>
//#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

class UdpChatClient : noncopyable
{
public:
	UdpChatClient(EventLoop* loop, const InetAddress& serverAddr)
		: client_(loop, serverAddr, "UdpChatClient"),
		codec_(std::bind(&UdpChatClient::onStringMessage, this, _1, _2, _3))
	{
		client_.setConnectionCallback(
			std::bind(&UdpChatClient::onConnection, this, _1));
		client_.setMessageCallback(
			std::bind(&UdpLengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
		//client_.enableRetry();
	}

	void connect()
	{
		client_.connect();
	}

	void disconnect()
	{
		client_.disconnect();
	}

	void write(const StringPiece& message)
	{
		//LOG_INFO << "into UdpChatClient::write";
		MutexLockGuard lock(mutex_);
		//std::unique_lock<std::mutex> lck(mutex_);
		if (connection_)
		{
			//LOG_INFO << "into UdpChatClient::write codec_.send(get_pointer(connection_), message);";
			codec_.send(get_pointer(connection_), message);
		}
	}

private:
	void onConnection(const UdpConnectionPtr& conn)
	{
		LOG_INFO << conn->localAddress().toIpPort() << " -> "
			<< conn->peerAddress().toIpPort() << " is "
			<< (conn->connected() ? "UP" : "DOWN");

		MutexLockGuard lock(mutex_);
		//std::unique_lock<std::mutex> lck(mutex_);
		if (conn->connected())
		{
			connection_ = conn;
		}
		else
		{
			connection_.reset();
		}
	}

	void onStringMessage(const UdpConnectionPtr&,
		const string& message,
		Timestamp)
	{
		printf("<<< %s\n", message.c_str());
	}

	//TcpClient client_;
	UdpClient client_;
	UdpLengthHeaderCodec codec_;
	MutexLock mutex_;
	//std::mutex mutex_;
	UdpConnectionPtr connection_;
};

#ifndef _WIN32
int main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
//int UdpChatClient_main(int argc, char* argv[])
#endif
{
	//LOG_INFO << "pid = " << getpid();
	if (argc > 2)
	{
		EventLoopThread loopThread;
		uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
		InetAddress serverAddr(argv[1], port);

		UdpChatClient client(loopThread.startLoop(), serverAddr);
		client.connect();
		std::string line;
		while (std::getline(std::cin, line))
		{
			client.write(line);
		}
		client.disconnect();
		//CurrentThread::sleepUsec(1000 * 1000);  // wait for disconnect, see ace/logging/client.cc
		// std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	else
	{
		EventLoopThread loopThread;
		//uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
		uint16_t port = 2000;
		InetAddress serverAddr("127.0.0.1", port);

		UdpChatClient client(loopThread.startLoop(), serverAddr);
		client.connect();
		std::string line;
		while (std::getline(std::cin, line))
		{
			client.write(line);
		}
		client.disconnect();
		//CurrentThread::sleepUsec(1000 * 1000);  // wait for disconnect, see ace/logging/client.cc
		// std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	//else
	//{
	//	printf("Usage: %s host_ip port\n", argv[0]);
	//}
	return 0;
}

