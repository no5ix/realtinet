#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "logging.h"
#include <sys/stat.h>
#include <sys/types.h>   
#include <io.h>

using namespace muduo;
using namespace muduo::net;

void newConnection(int sockfd,const InetAddress& peerAddr)
{
	LOG_INFO << "new connection from " << peerAddr.toIpPort();
	// can not use write in windows socket. use send 
	sockets::write(sockfd, "hello muduo\r\n", strlen("hello muduo\r\n"));
	sockets::close(sockfd);
	LOG_INFO << "close";
}

int main_acceptor()
{
	LOG_INFO << "main threadId = " << getCurrentThreadId();

	InetAddress listenAddr(2016);
	EventLoop loop;
	Acceptor acceptor(&loop, listenAddr, true);
	acceptor.setNewConnectionCallback(newConnection);
	acceptor.listen();
	loop.loop();

	return 0;
}
