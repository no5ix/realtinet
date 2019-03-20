#include "UdpConnection.h"

#include <base/Logging.h>
//#include <base/WeakCallback.h>
#include <net/Channel.h>
#include <net/EventLoop.h>
#include <net/Socket.h>
#include <net/SocketsOps.h>
#include "UdpSocketsOps.h"

#include <errno.h>

using namespace muduo;
using namespace muduo::net;

using kcpp::KcpSession;


void muduo::net::UdpDefaultConnectionCallback(const UdpConnectionPtr& conn)
{
	LOG_TRACE << conn->localAddress().toIpPort() << " -> "
		<< conn->peerAddress().toIpPort() << " is "
		<< (conn->connected() ? "UP" : "DOWN");
	// do not call conn->forceClose(), because some users want to register message callback only.
}

//void muduo::net::UdpDefaultMessageCallback(const UdpConnectionPtr&,
//	char*,
//	size_t,
//	Timestamp)
//{}

void muduo::net::UdpDefaultMessageCallback(const UdpConnectionPtr& conn,
	Buffer* buffer,
	Timestamp recvTime)
{}

UdpConnection::UdpConnection(const kcpp::RoleTypeE role,
	EventLoop* loop,
	const string& nameArg,
	Socket* connectedSocket,
	int ConnectionId,
	const InetAddress& localAddr,
	const InetAddress& peerAddr)
	:
	loop_(CHECK_NOTNULL(loop)),
	name_(nameArg),
	connId_(ConnectionId),
	state_(kConnecting),
	reading_(true),
	socket_(connectedSocket),
	channel_(new Channel(loop, socket_->fd())),
	localAddr_(localAddr),
	peerAddr_(peerAddr),
	//isCliKcpsessConned_(false),
	kcpSession_(new KcpSession(
		role,
		std::bind(&UdpConnection::DoSend, this, _1, _2),
		std::bind(&UdpConnection::DoRecv, this),
		[]() { return static_cast<int>(
		(Timestamp::now().microSecondsSinceEpoch() / 1000)); }))
{
	channel_->setReadCallback(
		std::bind(&UdpConnection::handleRead, this, _1));
	channel_->setCloseCallback(
		std::bind(&UdpConnection::handleClose, this));
	channel_->setErrorCallback(
		std::bind(&UdpConnection::handleError, this));
	LOG_DEBUG << "UdpConnection::ctor[" << name_ << "] at " << this
		<< " fd=" << socket_->fd();


	kcpSession_->setConnectionCallback(std::bind(&UdpConnection::onKcpsessConnection, this, _1));
}

void UdpConnection::onKcpsessConnection(std::deque<std::string>* pendingSendDataDeque)
{
	LOG_INFO << localAddress().toIpPort() << " -> "
		<< peerAddress().toIpPort() << " is "
		<< (kcpSession_->IsConnected() ? "UP" : "DOWN");
	if (kcpSession_->IsConnected())
		connectionCallback_(shared_from_this());
	else if (!kcpSession_->IsConnected())
		handleClose();
		//handlePendingSendDataDeque();
}

UdpConnection::~UdpConnection()
{
	assert(state_ == kDisconnected);
	LOG_DEBUG << "UdpConnection::dtor[" << name_ << "] at " << this
		<< " fd=" << channel_->fd()
		<< " state=" << stateToString();

	LOG_INFO << localAddress().toIpPort() << " -> "
		<< peerAddress().toIpPort() << " is "
		<< (connected() ? "UP" : "DOWN");

	loop_->cancel(curKcpsessUpTimerId_);
}

// FIXME efficiency!!!
void UdpConnection::send(Buffer* buf)
{
	if (state_ == kConnected)
	{
		if (loop_->isInLoopThread())
		{
			sendInLoop(buf->peek(), buf->readableBytes());
			buf->retrieveAll();
		}
		else
		{
			void (UdpConnection::*fp)(const StringPiece& message) = &UdpConnection::sendInLoop;
			loop_->runInLoop(
				std::bind(fp,
					this,     // FIXME
					buf->retrieveAllAsString()));
			//std::forward<string>(message)));
		}
	}
}

void UdpConnection::send(const StringPiece& message)
{
	if (state_ == kConnected)
	{
		if (loop_->isInLoopThread())
		{
			sendInLoop(message);
		}
		else
		{
			void (UdpConnection::*fp)(const StringPiece& message) = &UdpConnection::sendInLoop;
			loop_->runInLoop(
				std::bind(fp,
					this,     // FIXME
					message.as_string()));
			//std::forward<string>(message)));
		}
	}
}

void UdpConnection::sendInLoop(const StringPiece& message)
{
	sendInLoop(message.data(), message.size());
}

void UdpConnection::handleRead(Timestamp receiveTime)
{
	loop_->assertInLoopThread();
	int n = 0;
	while (kcpSession_->Recv(&kcpsessRcvBuf_, n))
	{
		//inputBuffer_.hasWritten(n);
		if (n < 0)
			LOG_ERROR << "kcpSession Recv() Error, Recv() = " << n;
		else if (n > 0)
		{
			inputBuffer_.append(kcpsessRcvBuf_.peek(), n);
			kcpsessRcvBuf_.retrieveAll();
			messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
			//messageCallback_(shared_from_this(), packetBuf_, n, receiveTime);
		}
	}
}

void UdpConnection::KcpSessionUpdate()
{
	auto kcpsessUpdateFunc = [&]() {
		auto nextUpdateTimeMs = kcpSession_->Update();
		if (nextUpdateTimeMs < 0)
		{
			handleClose();
			return;
		}
		curKcpsessUpTimerId_ = loop_->runAt(Timestamp(nextUpdateTimeMs * 1000), [&]() {
			KcpSessionUpdate();
			//if (!kcpSession_->CheckCanSend()/* || (kcpSession_->CheckTimeout() && kcpSession_->IsServer())*/)
			//	handleClose();
			//if (kcpSession_->IsClient() && !isCliKcpsessConned_ && kcpSession_->IsKcpsessConnected())
			//{
			//	isCliKcpsessConned_ = true;
			//	connectionCallback_(shared_from_this());
			//}
		});
	};

	if (loop_->isInLoopThread())
		kcpsessUpdateFunc();
	else
		loop_->runInLoop([&]() { kcpsessUpdateFunc(); });
}

void UdpConnection::DoSend(const void* data, int len)
{
	////if (state_ == kConnected)
	//{
	//	if (loop_->isInLoopThread())
	//	{
	//		sendInLoop(data, static_cast<size_t>(len));
	//	}
	//	else
	//	{
	//		void (UdpConnection::*fp)(const void* data, size_t len)
	//			= &UdpConnection::sendInLoop;
	//		loop_->runInLoop(
	//			std::bind(fp,
	//				this,     // FIXME
	//				data,
	//				static_cast<size_t>(len)));
	//	}
	//}
	ssize_t nwrote = 0;
	nwrote = sockets::write(channel_->fd(), data, len);
	if (nwrote >= 0)
	{
		if (len - nwrote == 0 && writeCompleteCallback_)
			loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
	}
	else // nwrote < 0
	{
		//LOG_SYSERR << "UdpConnection::DoSend";
		handleError();
	}
}

kcpp::UserInputData UdpConnection::DoRecv()
{
	int n = 0;
	n = sockets::read(channel_->fd(), static_cast<void*>(packetBuf_), kPacketBufSize);
	if (n == 0)
	{
		handleClose();
	}
	else if (n < 0)
	{
		//LOG_SYSERR << "UdpConnection::handleRead";
		handleError();
	}
	return kcpp::UserInputData(packetBuf_, n);
}

void UdpConnection::kcpSend(const void* data, int len,
	kcpp::TransmitModeE transmitMode /*= kcpp::TransmitModeE::kReliable*/)
{
	len = kcpSession_->Send(data, len, transmitMode);
	if (len < 0)
		LOG_ERROR << "kcpSession send failed";
}

void UdpConnection::sendInLoop(const void* data, size_t len)
{
	//LOG_INFO << "call sendInLoop";

	loop_->assertInLoopThread();
	if (state_ == kDisconnected)
	{
		LOG_WARN << "disconnected, give up writing";
		return;
	}
	kcpSend(data, len);
}

void UdpConnection::shutdown()
{
	// FIXME: use compare and swap
	if (state_ == kConnected)
	{
		setState(kDisconnecting);
	}
}

void UdpConnection::forceClose()
{
	// FIXME: use compare and swap
	if (state_ == kConnected || state_ == kDisconnecting)
	{
		setState(kDisconnecting);
		loop_->queueInLoop(std::bind(&UdpConnection::forceCloseInLoop, shared_from_this()));
	}
}

//void UdpConnection::forceCloseWithDelay(double seconds)
//{
//	if (state_ == kConnected || state_ == kDisconnecting)
//	{
//		setState(kDisconnecting);
//		loop_->runAfter(
//			seconds,
//			makeWeakCallback(shared_from_this(),
//				&UdpConnection::forceClose));  // not forceCloseInLoop to avoid race condition
//	}
//}

void UdpConnection::forceCloseInLoop()
{
	loop_->assertInLoopThread();
	if (state_ == kConnected || state_ == kDisconnecting)
	{
		// as if we received 0 byte in handleRead();
		handleClose();
	}
}

const char* UdpConnection::stateToString() const
{
	switch (state_)
	{
		case kDisconnected:
			return "kDisconnected";
		case kConnecting:
			return "kConnecting";
		case kConnected:
			return "kConnected";
		case kDisconnecting:
			return "kDisconnecting";
		default:
			return "unknown state";
	}
}

void UdpConnection::startRead()
{
	loop_->runInLoop(std::bind(&UdpConnection::startReadInLoop, this));
}

void UdpConnection::startReadInLoop()
{
	loop_->assertInLoopThread();
	if (!reading_ || !channel_->isReading())
	{
		channel_->enableReading();
		reading_ = true;
	}
}

void UdpConnection::stopRead()
{
	loop_->runInLoop(std::bind(&UdpConnection::stopReadInLoop, this));
}

void UdpConnection::stopReadInLoop()
{
	loop_->assertInLoopThread();
	if (reading_ || channel_->isReading())
	{
		channel_->disableReading();
		reading_ = false;
	}
}

void UdpConnection::connectEstablished()
{
	loop_->assertInLoopThread();
	assert(state_ == kConnecting);
	setState(kConnected);
	channel_->tie(shared_from_this());
	channel_->enableReading();

	KcpSessionUpdate();

	//if (kcpSession_->IsServer())
	//{
	//	//connectionCallback_(shared_from_this());
	//	handleRead(Timestamp::now()); // for the first recv data, see @firstRcvBuf_
	//}
}

void UdpConnection::connectDestroyed()
{
	loop_->assertInLoopThread();
	if (state_ == kConnected)
	{
		setState(kDisconnected);
		channel_->disableAll();

		connectionCallback_(shared_from_this());
	}
	channel_->remove();
}

void UdpConnection::handleClose()
{
	loop_->assertInLoopThread();
	LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToString();
	//assert(state_ == kConnected || state_ == kDisconnecting);
	// we don't close fd, leave it to dtor, so we can find leaks easily.
	setState(kDisconnected);
	channel_->disableAll();

	UdpConnectionPtr guardThis(shared_from_this());
	connectionCallback_(guardThis);
	// must be the last line
	closeCallback_(guardThis);
}

void UdpConnection::handleError()
{
	int err = sockets::getSocketError(channel_->fd());
	if (err != 0)
	{
		if (err == ECONNREFUSED)
		{
			LOG_INFO << peerAddr_.toIpPort() << " is disconnected";
		}
		else
		{
			LOG_ERROR << "UdpConnection::handleError [" << name_
				<< "] - SO_ERROR = " << err << " " << strerror_tl(err);
		}
		handleClose();
	}
}

