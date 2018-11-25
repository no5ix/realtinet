#ifndef CALM_NET_SOCKET_H_
#define CALM_NET_SOCKET_H_
#include "uncopyable.h"
namespace muduo
{
	namespace net
	{
		class InetAddress;

		class Socket :muduo::uncopyable
		{
		public:
			explicit Socket(int sockfd)
				:sockfd_(sockfd)
			{}
			~Socket();
			int fd() const { return sockfd_; }
			void bindAddress(const InetAddress& localaddr);
			void listen();
			int accept(InetAddress* peeraddr);

			void shutdownWrite();
			void setTcpNoDelay(bool on);
			void setReuseAddr(bool on);
			void setReusePort(bool on);
			void setKeepAlive(bool on);


		private:
			const int sockfd_;
		};
	}
}// end namespace muduo

#endif //CALM_NET_SOCKET_H_