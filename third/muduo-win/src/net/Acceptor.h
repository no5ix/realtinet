//////////////////////////////////////////////////////////////////////////
//
// Copyright(C)	2016, gjm_kevin.  All rights reserved.
// Author	:	gjm_kevin@163.com
// Date		:	2016.09.13
//
//////////////////////////////////////////////////////////////////////////
#ifndef CALM_NET_ACCEPTOR_H_
#define CALM_NET_ACCEPTOR_H_

#include <functional>
#include "uncopyable.h"


#include "Channel.h"
#include "Socket.h"

namespace muduo
{
	namespace net
	{
		class EventLoop;
		class InetAddress;

		class Acceptor :uncopyable
		{
		public:
			typedef std::function<void(int sockfd, const InetAddress&)> NewConnectionCallback;
			Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport = false);
			~Acceptor();
			void setNewConnectionCallback(const  NewConnectionCallback& cb)
			{
				NewConnectionCallback_ = cb;
			}
			bool listenning() const { return listenning_; }
			void listen();
		private:
			void handleRead();
			EventLoop* loop_;
			Socket acceptSocket_;
			Channel acceptChannel_;
			NewConnectionCallback NewConnectionCallback_;
			bool listenning_;
			int idleFd_;
		};// end class Acceptor

	}// end namespace net
}// end namespace muduo

#endif //CALM_NET_ACCEPTOR_H_