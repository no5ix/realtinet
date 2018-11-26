#include "SocketsOps.h"
#include "Socket.h"
#include "logging.h"
#include "InetAddress.h"

using namespace muduo;
using namespace muduo::net;

Socket::~Socket()
{
	sockets::close(sockfd_);
}
void Socket::bindAddress(const InetAddress& localaddr)
{
	sockets::bindOrDie(sockfd_,localaddr.getSockAddr());
}
void Socket::listen()
{
	sockets::listenOrDie(sockfd_);
}
int Socket::accept(InetAddress* peeraddr)
{
	struct sockaddr_in6 addr;
	memset(&addr, 0, sizeof(addr));
	int connfd = sockets::accept(sockfd_, &addr);
	if (connfd > 0)
	{
		peeraddr->setSockAddrInet6(addr);
	}
	return connfd;
}

void Socket::shutdownWrite()
{
	sockets::shutdownWrite(sockfd_);
}

void Socket::setTcpNoDelay(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval));
}
void Socket::setReuseAddr(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));
}
void Socket::setReusePort(bool on)
{
#ifdef SO_REUSEPORT
	int optval = on ? 1 : 0;
	int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
		&optval, static_cast<socklen_t>(sizeof optval));
	if (ret < 0 && on)
	{
		LOG_SYSERR << "SO_REUSEPORT failed.";
	}
#else
	if (on)
	{
		//LOG_ERROR << "SO_REUSEPORT is not supported.";
		;
	}
#endif
}

void Socket::setKeepAlive(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, (char*)&optval, sizeof(optval));
}