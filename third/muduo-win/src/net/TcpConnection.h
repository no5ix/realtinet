//////////////////////////////////////////////////////////////////////////
//
// Copyright(C)	2016, gjm_kevin.  All rights reserved.
// Author	:	gjm_kevin@163.com
// Date		:	2016.09.20
//
//////////////////////////////////////////////////////////////////////////
#ifndef CALM_NET_TCPCONNECTION_H_
#define CALM_NET_TCPCONNECTION_H_
#include "uncopyable.h"
#include "stringpiece.h"
#include "types.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"


namespace muduo
{
	namespace net
	{
		class EventLoop;
		class Channel;
		class Socket;

		class TcpConnection :muduo::uncopyable,
			public std::enable_shared_from_this<TcpConnection>
		{
		public:
			TcpConnection(EventLoop* loop,
				const string& name,
				int sockfd,
				const InetAddress& localAddr,
				const InetAddress& peerAddr);
			~TcpConnection();
			EventLoop* getLoop() const { return loop_; }
			const string& name() const { return name_; }
			const InetAddress& localAddress() const { return localAddr_; }
			const InetAddress& peerAddress() const { return peerAddr_; }
			bool connected() const { return state_ == kConnected; }
			bool disconnected()const { return state_ == kDisconnected; }

			void send(const void* message, int len);
			void send(const StringPiece& message);
			void send(Buffer* buf);

			void shutdown();
			void forceClose();
			//void forceCloseWithDelay(double seconds);
			void setTcpNoDelay(bool on);
			
			void startRead();
			void stopRead();
			bool isReading() const { return reading_; }

			void setConnectionCallback(const ConnectionCallback& cb)
			{
				connectionCallback_ = cb;
			}
			void setMessageCallback(const MessageCallback& cb)
			{
				messageCallback_ = cb;
			}
			void setWriteCompleteCallback(const WriteCompleteCallback& cb)
			{
				writeCompleteCallback_ = cb;
			}
			void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
			{
				highWaterMark_ = highWaterMark;
				highWaterMarkCallback_ = cb;
			}
			void setCloseCallback(const CloseCallback& cb)
			{
				closeCallback_ = cb;
			}
			Buffer* inputBuffer()
			{
				return &inputBuffer_;
			}
			Buffer* outputBuffer()
			{
				return &outputBuffer_;
			}


			// called when TcpServer accepts a new connection
			void connectEstablished();   // should be called only once
										 // called when TcpServer has removed me from its map
			void connectDestroyed();  // should be called only once

		private:
			enum StateE{kDisconnected,kConnecting,kConnected,kDisconnecting};
			void handleRead(Timestamp receiveTime);
			void handleWrite();
			void handleClose();
			void handleError();

			void sendInLoop(const StringPiece& message);
			void sendInLoop(const void* message, size_t len);

			void shutdownInLoop();
			void forceCloseInLoop();

			void setState(StateE s) { state_ = s; }

			const char* stateToString() const;

			void startReadInLoop();
			void stopReadInLoop();

			EventLoop* loop_;
			const string name_;
			StateE state_;
			std::shared_ptr<Socket> socket_;
			std::shared_ptr<Channel> channel_;
			const InetAddress localAddr_;
			const InetAddress peerAddr_;
			ConnectionCallback connectionCallback_;
			MessageCallback messageCallback_;
			WriteCompleteCallback writeCompleteCallback_;
			HighWaterMarkCallback highWaterMarkCallback_;
			CloseCallback closeCallback_;
			size_t highWaterMark_;
			Buffer inputBuffer_;
			Buffer outputBuffer_;
			bool reading_;
		};//end class TcpConnection
	}//end namespace net
}//end namespace muduo 

#endif //CALM_NET_TCPCONNECTION_H_