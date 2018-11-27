#include <net/SocketsOps.h>
#include "UdpSocketsOps.h"

#include <base/Logging.h>
#include <base/Types.h>
#include <net/Endian.h>
#include <net/Buffer.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>  // snprintf

#ifndef _WIN32

#include <strings.h>  // bzero
#include <sys/socket.h>
#include <sys/uio.h>  // readv
#include <unistd.h>

#else

#include <stdlib.h>

#pragma comment( lib, "ws2_32.lib"  )  // for select 
#pragma comment(lib,"ws2_32")
#include <WinSock2.h>

#endif


#ifdef _MSC_VER
#define snprintf	sprintf_s
#endif

using namespace muduo;
using namespace muduo::net;

#ifndef _WIN32

namespace
{

typedef struct sockaddr SA;



#if VALGRIND || defined (NO_ACCEPT4)
void setNonBlockAndCloseOnExec(int sockfd)
{
	// non-block
	int flags = ::fcntl(sockfd, F_GETFL, 0);
	flags |= O_NONBLOCK;
	int ret = ::fcntl(sockfd, F_SETFL, flags);
	// FIXME check

	// close-on-exec
	flags = ::fcntl(sockfd, F_GETFD, 0);
	flags |= FD_CLOEXEC;
	ret = ::fcntl(sockfd, F_SETFD, flags);
	// FIXME check

	(void)ret;
}
#endif

}

int sockets::createUdpNonblockingOrDie(sa_family_t family)
{
#if VALGRIND
	int sockfd = ::socket(family, SOCK_DGRAM, IPPROTO_UDP);

	if (sockfd < 0)
	{
		LOG_SYSFATAL << "sockets::createUdpNonblockingOrDie";
	}

	setNonBlockAndCloseOnExec(sockfd);
#else
	int sockfd = ::socket(family, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_UDP);
	if (sockfd < 0)
	{
		LOG_SYSFATAL << "sockets::createUdpNonblockingOrDie";
	}
#endif
	return sockfd;
}

#else

void setNonBlock(int sockfd)
{
#ifdef _MSC_VER
	u_long nonblock = 1;
	int ret = ioctlsocket(sockfd, FIONBIO, &nonblock);
#else
	int ret = fcntl(fd, F_SETFL, O_NONBLOCK | fcntl(fd, F_GETFL));
#endif
	if (ret == SOCKET_ERROR)
	{
		LOG_ERROR << "set no block error";
	}
}

int sockets::createUdpNonblockingOrDie(int family)
{
	int sockfd = socket(family, SOCK_DGRAM, 0);
	int saveErrno = GetLastError();
	if (sockfd < 0)
	{
		errno = saveErrno;
		LOG_SYSFATAL << "sockets::createUdpNonblockingOrDie";
	}
	setNonBlock(sockfd);
	return sockfd;
}

#endif

//////////
// for udp socket
/////////
int sockets::recvfrom(int sockfd, struct sockaddr_in6* addr, Buffer *recvfromBuf)
{
	const uint16_t MAX_PACKET_BYTE_LENGTH = 512;

	char packetMem[MAX_PACKET_BYTE_LENGTH];
	int packetSize = sizeof(packetMem);
	socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);

	int readByteCount = static_cast<int>(::recvfrom(sockfd,
		packetMem,
		packetSize,
		0,
		sockaddr_cast(addr),
		&addrlen));


	if (readByteCount < 0)
	{
		int savedErrno = errno;
		LOG_SYSERR << "Socket::accept";
		switch (savedErrno)
		{
			case EAGAIN:
				// expected errors
				errno = savedErrno;
				break;
			default:
				LOG_FATAL << "unknown error of ::recvfrom " << savedErrno;
				break;
		}
	}
	recvfromBuf->append(packetMem, readByteCount);
	return readByteCount;
}
