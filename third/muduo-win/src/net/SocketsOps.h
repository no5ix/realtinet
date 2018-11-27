#ifndef CALM_NET_SOCKETSOPS_H_
#define CALM_NET_SOCKETSOPS_H_

#include <stdint.h>
#include <stdio.h>

#ifdef _MSC_VER
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
// sockets
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif // _MSC_VER
#ifndef _MSC_VER
#define IOV_TYPE struct iovec
#define IOV_PTR_FIELD iov_base
#define IOV_LEN_FIELD iov_len
#define IOV_LEN_TYPE size_t
#else
#define NUM_WRITE_IOVEC 16
#define IOV_TYPE WSABUF
#define IOV_PTR_FIELD buf
#define IOV_LEN_FIELD len
#define IOV_LEN_TYPE unsigned long
#endif
typedef struct _stPipe
{
	int pipe_read;
	int pipe_write;
}stPipe;

namespace muduo
{
	namespace net
	{
		namespace sockets
		{
			int createNoneblockOrDie(int family);
			int createOrDie(int family);
			int connect(int socketfd, const struct sockaddr* addr);
			void bindOrDie(int sockfd, const struct sockaddr* addr);
			void listenOrDie(int sockfd);
			
			int accept(int sockfd, struct sockaddr_in6* addr);

			ssize_t read(int sockfd, void *buf, size_t count);
			ssize_t write(int sockfd, const void* buf, size_t count);
			ssize_t readv(int sockfd, IOV_TYPE *iov, int iovcnt);

			void close(int sockfd);
			void shutdownWrite(int sockfd);
			void toIpPort(char* buf, size_t size,const struct sockaddr* addr);
			void toIp(char* buf, size_t size, const struct sockaddr* addr);

			void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);

			int getSocketError(int sockfd);

			//const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr) ;
			//const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr) ;
			//struct sockaddr* sockaddr_cast(struct sockaddr_in* addr);
			//struct sockaddr_in* sockaddr_in_cast(struct sockaddr* addr);

			const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
			const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
			struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr);
			struct sockaddr* sockaddr_cast(struct sockaddr_in* addr);
			const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);
			const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr);

			struct sockaddr_in6 getLocalAddr(int sockfd);
			struct sockaddr_in6 getPeerAddr(int sockfd);

			bool isSelfConnect(int sockfd);
			int pipe(int fildes[2]);
			stPipe pipe();
		}// end namespace sockets
	}// end namespace net
}// end namespace muduo

#endif //CALM_NET_SOCKETSOPS_H_