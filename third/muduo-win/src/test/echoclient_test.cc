//#include <net/TcpClient.h>
//
//#include <base/Logging.h>
//#include <base/Thread.h>
//#include <net/EventLoop.h>
//#include <net/InetAddress.h>



//#include "EventLoopThread.h"

#include <base/logging.h>
//#include <mutex>
//#include "logging.h""
#include <net/TcpClient.h>

//#include <base/thread.h>
#include <net/EventLoop.h>
//#include <net/InetAddress.h>

//#include <utility>

//#include <stdio.h>
//#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

int numThreads = 0;
class EchoClient;
std::vector<std::unique_ptr<EchoClient>> clients;
int current = 0;

class EchoClient
{
public:
	EchoClient(EventLoop* loop, const InetAddress& listenAddr, const string& id)
		: loop_(loop),
		client_(loop, listenAddr, "EchoClient" + id)
	{
		client_.setConnectionCallback(
			std::bind(&EchoClient::onConnection, this, std::placeholders::_1));
		client_.setMessageCallback(
			std::bind(&EchoClient::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		//client_.enableRetry();
	}

	void connect()
	{
		client_.connect();
	}
	// void stop();

private:
	void onConnection(const TcpConnectionPtr& conn)
	{
		//LOG_TRACE << conn->localAddress().toIpPort() << " -> "
		//	<< conn->peerAddress().toIpPort() << " is "
		//	<< (conn->connected() ? "UP" : "DOWN");

		if (conn->connected())
		{
			++current;
			if (implicit_cast<size_t>(current) < clients.size())
			{
				clients[current]->connect();
			}
			//LOG_INFO << "*** connected " << current;
		}
		conn->send("world ");
	}

	void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
	{
		string msg(buf->retrieveAllAsString());
		//LOG_TRACE << conn->name() << " recv " << msg.size() << " bytes at " << time.toString();
		LOG_INFO << conn->name() << " recv " << msg;

		if (msg == "quit\n")
		{
			conn->send("bye\n");
			conn->shutdown();
		}
		else if (msg == "shutdown\n")
		{
			loop_->quit();
		}
		else
		{
			conn->send(msg);
		}
	}

	EventLoop* loop_;
	TcpClient client_;
};

void massin(int argc, char* argv[])
//int main(int argc, char* argv[])
{
	//LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
	//if (argc > 1)
	{
		EventLoop loop;
		//bool ipv6 = argc > 3;
		InetAddress serverAddr("127.0.0.1", 2000);

		int n = 5;
		if (argc > 2)
		{
			n = atoi(argv[2]);
		}

		clients.reserve(n);
		for (int i = 0; i < n; ++i)
		{
			char buf[32];
			snprintf(buf, sizeof buf, "%d", i + 1);
			clients.emplace_back(new EchoClient(&loop, serverAddr, buf));
		}

		clients[current]->connect();
		loop.loop();
	}
	//else
	//{
	//	printf("Usage: %s host_ip [current#]\n", argv[0]);
	//}
}

