//////////////////////////////////////////////////////////////////////////
//
// Copyright(C)	2016, gjm_kevin.  All rights reserved.
// Author	:	gjm_kevin@163.com
//
//////////////////////////////////////////////////////////////////////////
#ifndef CALM_NET_TCPSERVER_H_
#define CALM_NET_TCPSERVER_H_
#pragma warning(disable : 4996)

#include "SocketsOps.h"
#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "logging.h"
#include "Acceptor.h"
#include "TcpConnection.h"

#include "uncopyable.h"
#include "types.h"

#include <map>
#include <memory>
#include <functional>

namespace muduo
{
	namespace net
	{
		class Acceptor;
		class EventLoop;
		class EventLoopThreadPool;
		///
		/// TCP server, supports single-threaded and thread-pool models.
		///
		/// This is an interface class, so don't expose too much details.
		class TcpServer :muduo::uncopyable
		{
		public:
			typedef std::function<void(EventLoop*)> ThreadInitCallback;
			enum Option
			{
				kNoReusePort,
				kReusePort,
			};
			TcpServer(EventLoop* loop,
				const InetAddress& listenAddr,
				const string& nameArg,
				Option option = kNoReusePort);
			~TcpServer();
			const string& ipPort()const { return ipPort_; }
			EventLoop* getLoop() const { return loop_; }

			///set the number of threads for handling input.
			///
			/// Always accepts new connection in loop's thread.
			/// Must be called before @c start
			/// @param numThreads
			/// - 0 means all I/O in loop's thread, no thread will created.
			///   this is the default value.
			/// - 1 means all I/O in another thread.
			/// - N means a thread pool with N threads, new connections
			///   are assigned on a round-robin basis.
			void setThreadNum(int numThreads);
			void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }

			std::shared_ptr<EventLoopThreadPool> threadPool() { return threadPool_; }

			///Starts the server if it's not listening.
			///It's harmless to call it multiple times.
			///Thread safe
			void start();

			/// set callbacks
			void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
			void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
			void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
		private:

			///Not thread safe, but in loop
			void newConnection(int sockfd, const InetAddress& peerAddr);
			///Thread safe
			void removeConnection(const TcpConnectionPtr& conn);
			///Not thread safe, but in loop
			void removeConnectionInLoop(const TcpConnectionPtr& conn);

			typedef std::map<string, TcpConnectionPtr>  ConnectionMap;

			EventLoop * loop_;
			const string ipPort_;
			const string name_;
			std::shared_ptr<Acceptor> acceptor_;
			std::shared_ptr<EventLoopThreadPool> threadPool_;
			ConnectionCallback connectionCallback_;
			MessageCallback messageCallback_;
			WriteCompleteCallback writeCompleteCallback_;
			ThreadInitCallback threadInitCallback_;
			int nextConnId_;
			bool started_;
			ConnectionMap connections_;


		};// end class TcpServer
	}// end namespace net
}// end namespace muduo

#endif //CALM_NET_TCPSERVER_H_