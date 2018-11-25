#include "EventLoop.h"
#include "Acceptor.h"
#include "logging.h"
#include "InetAddress.h"
#include "SocketsOps.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>   
#include <io.h>

using namespace muduo;
using namespace muduo::net;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport /* = false */)
	:loop_(loop),
	acceptSocket_(sockets::createNoneblockOrDie(listenAddr.family())),
	acceptChannel_(loop, acceptSocket_.fd()),
	listenning_(false),
	idleFd_(::_open("nul", O_RDONLY))
{
	assert(idleFd_ >= 0);
	acceptSocket_.setReuseAddr(true);
	acceptSocket_.setReusePort(reuseport);
	acceptSocket_.bindAddress(listenAddr);
	acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
	acceptChannel_.disableAll();
	acceptChannel_.remove();
	::_close(idleFd_);
}
void Acceptor::listen()
{
	loop_->assertInLoopThread();
	listenning_ = true;
	acceptSocket_.listen();
	acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
	loop_->assertInLoopThread();
	InetAddress peerAddr;
	int connfd = acceptSocket_.accept(&peerAddr);
	int saveErrno = GetLastError();
	if (connfd > 0)
	{
		if (NewConnectionCallback_)
		{
			NewConnectionCallback_(connfd, peerAddr);
		}
		else
		{
			sockets::close(connfd);
		}
	}
	else
	{
		errno = saveErrno;
		LOG_SYSERR << "in Acceptor::handleRead";
		//do not have enough file descriptor resource
		if (errno == WSAEMFILE)
		{
			::_close(idleFd_);
			idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
			::_close(idleFd_);
			idleFd_ = ::_open("nul", O_RDONLY);
		}
	}
}
