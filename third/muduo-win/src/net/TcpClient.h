//////////////////////////////////////////////////////////////////////////
//
// Copyright(C)	2016, gjm_kevin.  All rights reserved.
// Author	:	gjm_kevin@163.com
//
//////////////////////////////////////////////////////////////////////////
#ifndef CALM_NET_TCPCLIENT_H_
#define CALM_NET_TCPCLIENT_H_
#include "uncopyable.h"
#include <mutex>
#include "TcpConnection.h"

namespace muduo
{
	namespace net
	{
		class Connector;
		typedef std::shared_ptr<Connector> ConnectorPtr;

		class TcpClient :muduo::uncopyable
		{
		public:
			TcpClient(EventLoop*loop,
				const InetAddress& serverAddr,
				const string& nameArg);
			~TcpClient();

			void connect();
			void disconnect();
			void stop();

			TcpConnectionPtr connection() const
			{
				std::unique_lock<std::mutex> lck(mutex_);
				return connection_;
			}
			
			EventLoop* getLoop() const { return loop_; }
			bool retry() const { return retry_; }
			void enableRetry() { retry_ = true; }
			const string& name() const { return name_; }

			/// Not thread safe.
			void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
			void setMessageCallback(const MessageCallback& cb) {  messageCallback_ = cb; }
			void setWriteCompleteCallback_(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
		private:
			/// Not thread safe, but in loop
			void newConnection(int sockfd);
			/// Not thread safe, but in loop
			void removeConnection(const TcpConnectionPtr& conn);

			EventLoop* loop_;
			ConnectorPtr connector_;
			const string name_;
			ConnectionCallback connectionCallback_;
			MessageCallback messageCallback_;
			WriteCompleteCallback writeCompleteCallback_;
			bool retry_;
			bool connect_;
			int nextConnId_;
			mutable std::mutex mutex_;
			TcpConnectionPtr connection_;

		};//end class TcpClient
	}// end namespace net
}// end namespace muduo

#endif //CALM_NET_TCPCLIENT_H_