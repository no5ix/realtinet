#pragma once


//#ifndef MUDUO_EXAMPLES_ASIO_CHAT_CODEC_H
//#define MUDUO_EXAMPLES_ASIO_CHAT_CODEC_H


// #include <base/logging.h>
#include <base/Logging.h>
#include <net/Buffer.h>
#include <net/Endian.h>
//#include <net/TcpConnection.h>
#include <core/UdpConnection.h>

class UdpLengthHeaderCodec{
public:
//	typedef std::function<void(const muduo::net::TcpConnectionPtr&,
//		const muduo::string& message,
//		muduo::Timestamp)> TcpStringMessageCallback;
//
//	explicit UdpLengthHeaderCodec(const TcpStringMessageCallback& cb)
//		: tcp_messageCallback_(cb)
//	{}
//
//	void onMessage(const muduo::net::TcpConnectionPtr& conn,
//		muduo::net::Buffer* buf,
//		muduo::Timestamp receiveTime)
//	{
//		//LOG_INFO << "into UdpLengthHeaderCodec::onMessage";
//
//		while (buf->readableBytes() >= kHeaderLen) // kHeaderLen == 4
//		{
//			// FIXME: use Buffer::peekInt32()
//			const void* data = buf->peek();
//			int32_t be32 = *static_cast<const int32_t*>(data); // SIGBUS
//			const int32_t len = muduo::net::sockets::networkToHost32(be32);
//			if (len > 65536 || len < 0)
//			{
//				LOG_ERROR << "Invalid length " << len;
//				conn->shutdown();  // FIXME: disable reading
//				break;
//			}
//			else if (buf->readableBytes() >= len + kHeaderLen)
//			{
//				buf->retrieve(kHeaderLen);
//				muduo::string message(buf->peek(), len);
//				tcp_messageCallback_(conn, message, receiveTime);
//				//LOG_INFO << "into UdpLengthHeaderCodec::onMessage messageCallback_(conn, message, receiveTime);";
//				buf->retrieve(len);
//			}
//			else
//			{
//				break;
//			}
//		}
//	}
//
//	// FIXME: TcpConnectionPtr
//	void send(muduo::net::TcpConnection* conn,
//		const muduo::StringPiece& message)
//	{
//		muduo::net::Buffer buf;
//		buf.append(message.data(), message.size());
//		int32_t len = static_cast<int32_t>(message.size());
//		int32_t be32 = muduo::net::sockets::hostToNetwork32(len);
//		buf.prepend(&be32, sizeof be32);
//		conn->send(&buf);
//	}



	typedef std::function<void(const muduo::net::UdpConnectionPtr&,
		const muduo::string& message,
		muduo::Timestamp)> UdpStringMessageCallback;

	explicit UdpLengthHeaderCodec(const UdpStringMessageCallback& cb)
		: udp_messageCallback_(cb)
	{}

	void onMessage(const muduo::net::UdpConnectionPtr& conn,
		muduo::net::Buffer* buf,
		muduo::Timestamp receiveTime)
	{
		//LOG_INFO << "into UdpLengthHeaderCodec::onMessage";

		while (buf->readableBytes() >= kHeaderLen) // kHeaderLen == 4
		{
			// FIXME: use Buffer::peekInt32()
			const void* data = buf->peek();
			int32_t be32 = *static_cast<const int32_t*>(data); // SIGBUS
			const int32_t len = muduo::net::sockets::networkToHost32(be32);
			if (len > 65536 || len < 0)
			{
				LOG_ERROR << "Invalid length " << len;
				conn->shutdown();  // FIXME: disable reading
				break;
			}
			else if (buf->readableBytes() >= len + kHeaderLen)
			{
				buf->retrieve(kHeaderLen);
				muduo::string message(buf->peek(), len);
				udp_messageCallback_(conn, message, receiveTime);
				//LOG_INFO << "into UdpLengthHeaderCodec::onMessage messageCallback_(conn, message, receiveTime);";
				buf->retrieve(len);
			}
			else
			{
				break;
			}
		}
	}

	// FIXME: UdpConnectionPtr
	void send(muduo::net::UdpConnection* conn,
		const muduo::StringPiece& message)
	{
		muduo::net::Buffer buf;
		buf.append(message.data(), message.size());
		int32_t len = static_cast<int32_t>(message.size());
		int32_t be32 = muduo::net::sockets::hostToNetwork32(len);
		buf.prepend(&be32, sizeof be32);
		conn->send(&buf);
	}

private:
	//TcpStringMessageCallback tcp_messageCallback_;
	UdpStringMessageCallback udp_messageCallback_;
	const static size_t kHeaderLen = sizeof(int32_t);
};

//#endif  // MUDUO_EXAMPLES_ASIO_CHAT_CODEC_H
