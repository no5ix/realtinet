#pragma warning(disable : 4996)
#include "EventLoopThread.h"
#include "logging.h"
#include "TcpClient.h"
#include <thread>

using namespace muduo;
using namespace muduo::net;


//int main()
int main_tcpclient()
{
	Logger::setLogLevel(Logger::DEBUG);

	EventLoopThread loopThread;
	{
		InetAddress serverAddr("127.0.0.1", 2016); // should succeed
		TcpClient client(loopThread.startLoop(), serverAddr, "TcpClient");
		client.connect();
		std::this_thread::sleep_for(std::chrono::seconds(5));
		client.disconnect();
	}
	std::this_thread::sleep_for(std::chrono::seconds(3));
	
	
	return 0;
}
