#include <SocketsOps.h>
#include <logging.h>
#include <types.h>
#include <Endian.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment( lib, "ws2_32.lib"  )  // for select 
#pragma comment(lib,"ws2_32")
#include <WinSock2.h>

#ifdef _MSC_VER
#define snprintf	sprintf_s
#endif

using namespace muduo;
using namespace muduo::net;

namespace
{
	class InitSocket
	{
	public:
		InitSocket()
		{
#ifdef _MSC_VER
			WSADATA  Ws;
			//Init Windows Socket
			if (WSAStartup(MAKEWORD(2, 2), &Ws) != 0)
			{
				errno = GetLastError();
				LOG_SYSFATAL << "WSAStartup failed!";
			}
			else
			{
				LOG_INFO << "WSAStartup success!";
			}
		
#else
			//Linux not need to init socket
#endif
		}

	};// end class IgnoreSigPipe
	InitSocket initWSA;
}

namespace
{
	typedef struct sockaddr SA;

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

	void setNonDelay(int sockfd)
	{
		BOOL nodelay = TRUE;
		int ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));
		if (ret == SOCKET_ERROR)
		{
			LOG_ERROR << "set no delay error";
		}
	}
} // end anonymous namespace

//const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in * addr)
//{
//	return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
//}
//
//const struct sockaddr_in* sockets::sockaddr_in_cast(const struct sockaddr * addr) 
//{
//	return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
//}
//
//struct sockaddr* sockets::sockaddr_cast(struct sockaddr_in * addr)
//{
//	return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
//}
//
//struct sockaddr_in* sockets::sockaddr_in_cast(struct sockaddr * addr)
//{
//	return static_cast<struct sockaddr_in*>(implicit_cast<void*>(addr));
//}

const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in6* addr)
{
	return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

struct sockaddr* sockets::sockaddr_cast(struct sockaddr_in6* addr)
{
	return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}

const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in* addr)
{
	return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

struct sockaddr* sockets::sockaddr_cast(struct sockaddr_in* addr)
{
	return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}

const struct sockaddr_in* sockets::sockaddr_in_cast(const struct sockaddr* addr)
{
	return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}

const struct sockaddr_in6* sockets::sockaddr_in6_cast(const struct sockaddr* addr)
{
	return static_cast<const struct sockaddr_in6*>(implicit_cast<const void*>(addr));
}

int sockets::createOrDie(int family)
{
	int sockfd = socket(family, SOCK_STREAM, 0);
	int saveErrno = GetLastError();
	if (sockfd < 0)
	{
		errno = saveErrno;
		LOG_SYSFATAL << "sockets::createOrDie";
	}
	return sockfd;
}


int sockets::createNoneblockOrDie(int family)
{
	int sockfd = socket(family, SOCK_STREAM, 0);
	int saveErrno = GetLastError();
	if (sockfd < 0)
	{
		errno = saveErrno;
		LOG_SYSFATAL << "sockets::createNoneblockOrDie";
	}
	setNonBlock(sockfd);
	return sockfd;
}

void sockets::bindOrDie(int sockfd, const  sockaddr * addr)
{
	int ret = ::bind(sockfd, addr, sizeof(struct sockaddr));
	int saveErrno = GetLastError();
	if (ret < 0)
	{
		errno = saveErrno;
		LOG_SYSFATAL << "sockets::bindOrDie";
	}
}
void sockets::listenOrDie(int sockfd)
{
	int ret = listen(sockfd, SOMAXCONN);
	int saveErrno = GetLastError();
	if (ret < 0)
	{
		errno = saveErrno;
		LOG_SYSFATAL << "sockets::listenOrDie";
	}
}

int sockets::accept(int sockfd, struct sockaddr_in6* addr)
{
	socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
	int connfd =::accept(sockfd,sockaddr_cast(addr),&addrlen);
	setNonBlock(connfd);
	if (connfd == INVALID_SOCKET)
	{
		errno = GetLastError();
		LOG_FATAL << "error sockets::accept";
	}
	return connfd;
}

int sockets::connect(int socketfd, const struct sockaddr* addr)
{
	return ::connect(socketfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr)));
}

ssize_t sockets::read(int sockfd, void *buf, int count)
{
	return recv(sockfd, static_cast<char*>(buf),count,0);
}

ssize_t sockets::write(int sockfd, const void* buf, int count)
{
	return send(sockfd,static_cast<const char*>(buf), count,0);
}


ssize_t sockets::readv(int sockfd, IOV_TYPE *iov, int iovcnt)
{

#ifdef _MSC_VER
	//WSABUF vecs[4];
	DWORD bytesRead;
	DWORD flags = 0;
	if (WSARecv(sockfd, iov, iovcnt, &bytesRead, &flags, NULL, NULL)) {
		/* The read failed. It might be a close,
		* or it might be an error. */
		if (GetLastError() == WSAECONNABORTED)
			return  0;
		else
			return -1;
	}
	else
		return bytesRead;
#else
	return ::readv(sockfd, iov, iovcnt);
#endif
}



void sockets::close(int sockfd)
{
	if (::closesocket(sockfd) < 0)
	{
		errno = GetLastError();
		LOG_SYSERR << "sockets::close";
	}
}

void sockets::shutdownWrite(int sockfd)
{
	if (::shutdown(sockfd, SD_SEND) < 0)
	{
		errno = GetLastError();
		LOG_SYSERR << "sockets::shutdownWrite";
	}
}
void sockets::toIp(char* buf, size_t size,const struct sockaddr* addr)
{
	if (addr->sa_family == AF_INET)
	{
		const struct sockaddr_in * addr4 = sockaddr_in_cast(addr);
		::inet_ntop(AF_INET, (PVOID)&addr4->sin_addr, buf, static_cast<socklen_t>(size));
	}
}
void sockets::toIpPort(char* buf, size_t size,const struct sockaddr* addr)
{
	toIp(buf, size, addr);
	size_t end = strlen(buf);
	const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
	uint16_t port = sockets::networkToHost16(addr4->sin_port);
	snprintf(buf + end, size - end, ":%u", port);
}

void sockets::fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr)
{
	addr->sin_family = AF_INET;
	addr->sin_port = hostToNetwork16(port);
	if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
	{
		errno = GetLastError();
		LOG_SYSERR << "sockets::fromIpPort";
	}
}

int sockets::getSocketError(int sockfd)
{
	int optval;
	socklen_t optlen = static_cast<socklen_t>(sizeof(optval));
	if (::getsockopt(sockfd,SOL_SOCKET,SO_ERROR, (char*)&optval,&optlen) < 0)
	{
		return errno;
	}
	else
	{
		return optval;
	}
}
struct sockaddr_in6 sockets::getLocalAddr(int sockfd)
{
	struct sockaddr_in6 localaddr;
	memset(&localaddr, 0, sizeof(localaddr));
	socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
	if(::getsockname(sockfd,sockaddr_cast(&localaddr),&addrlen) < 0)
	{
		errno = GetLastError();
		LOG_SYSERR << "sockets::getLocalAddr";
	}
	return localaddr;
}
struct sockaddr_in6 sockets::getPeerAddr(int sockfd)
{
	struct sockaddr_in6 peeraddr;
	memset(&peeraddr, 0, sizeof(peeraddr));
	socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
	if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0)
	{
		errno = GetLastError();
		LOG_SYSERR << "sockets::getLocalAddr";
	}
	return peeraddr;
}

//bool sockets::isSelfConnect(int sockfd)
//{
//	struct sockaddr_in6 localaddr = getLocalAddr(sockfd);
//	struct sockaddr_in6 peeraddr = getPeerAddr(sockfd);
//	return localaddr.sin6_addr == peeraddr.sin6_addr
//		&& localaddr.sin6_port == peeraddr.sin6_port;
//}

bool sockets::isSelfConnect(int sockfd)
{
	struct sockaddr_in6 localaddr = getLocalAddr(sockfd);
	struct sockaddr_in6 peeraddr = getPeerAddr(sockfd);
	if (localaddr.sin6_family == AF_INET)
	{
		const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localaddr);
		const struct sockaddr_in* raddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr);
		return laddr4->sin_port == raddr4->sin_port
			&& laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
	}
	else if (localaddr.sin6_family == AF_INET6)
	{
		return localaddr.sin6_port == peeraddr.sin6_port
			&& memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof localaddr.sin6_addr) == 0;
	}
	else
	{
		return false;
	}
}

// windows pipe use tcp 
//refer to http://blog.csdn.net/chief1985/article/details/5064998
int sockets::pipe(int fildes[2])
{
	int tcp1, tcp2;
	sockaddr_in name;
	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	int namelen = sizeof(name);
	tcp1 = tcp2 = -1;
	int tcp = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp == -1) {
		goto clean;
	}
	if (bind(tcp, (sockaddr*)&name, namelen) == -1) {
		goto clean;
	}
	if (listen(tcp, 5) == -1) {
		goto clean;
	}
	if (getsockname(tcp, (sockaddr*)&name, &namelen) == -1) {
		goto clean;
	}
	tcp1 = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp1 == -1) {
		goto clean;
	}
	if (-1 == connect(tcp1, (sockaddr*)&name, namelen)) {
		goto clean;
	}
	tcp2 = accept(tcp, (sockaddr*)&name, &namelen);
	if (tcp2 == -1) {
		goto clean;
	}
	if (closesocket(tcp) == -1) {
		goto clean;
	}
	fildes[0] = tcp1;
	fildes[1] = tcp2;
	return 0;
clean:
	if (tcp != -1) {
		closesocket(tcp);
	}
	if (tcp2 != -1) {
		closesocket(tcp2);
	}
	if (tcp1 != -1) {
		closesocket(tcp1);
	}
	return -1;
}

stPipe sockets::pipe()
{
	stPipe sock_pipe;
	sock_pipe.pipe_read = -1;
	sock_pipe.pipe_write = -1;
	int tcp1, tcp2;
	sockaddr_in name;
	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	int namelen = sizeof(name);
	tcp1 = tcp2 = -1;
	int tcp = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp == -1) {
		goto clean;
	}
	if (bind(tcp, (sockaddr*)&name, namelen) == -1) {
		goto clean;
	}
	if (listen(tcp, 5) == -1) {
		goto clean;
	}
	if (getsockname(tcp, (sockaddr*)&name, &namelen) == -1) {
		goto clean;
	}
	tcp1 = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp1 == -1) {
		goto clean;
	}
	if (-1 == connect(tcp1, (sockaddr*)&name, namelen)) {
		goto clean;
	}
	tcp2 = accept(tcp, (sockaddr*)&name, &namelen);
	if (tcp2 == -1) {
		goto clean;
	}
	if (closesocket(tcp) == -1) {
		goto clean;
	}
	sock_pipe.pipe_read = tcp1;
	sock_pipe.pipe_write = tcp2;
	return sock_pipe;
clean:
	if (tcp != -1) {
		closesocket(tcp);
	}
	if (tcp2 != -1) {
		closesocket(tcp2);
	}
	if (tcp1 != -1) {
		closesocket(tcp1);
	}
	return sock_pipe;
}
