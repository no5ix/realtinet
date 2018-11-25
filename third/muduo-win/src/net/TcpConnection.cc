//////////////////////////////////////////////////////////////////////////
//
// Copyright(C)	2016, gjm_kevin.  All rights reserved.
// Author	:	gjm_kevin@163.com
// Date		:	2016.09.20
//
//////////////////////////////////////////////////////////////////////////
#pragma warning(disable : 4996)
#include "EventLoop.h"
#include "Socket.h"
#include "SocketsOps.h"
#include "logging.h"
#include "TcpConnection.h"
#include "Channel.h"

#include <functional>
#include <errno.h>


using namespace muduo;
using namespace muduo::net;

void muduo::net::defaultConnectionCallback(const TcpConnectionPtr& conn)
{
	LOG_TRACE << conn->localAddress().toIpPort() << " -> "
		<< conn->peerAddress().toIpPort() << " is "
		<< (conn->connected() ? "UP" : "DOWN");	
}
void muduo::net::defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp receiveTime)
{
	//discard all data
	buffer->retrieveAll();
}
TcpConnection::TcpConnection(EventLoop* loop,
	const string& name,
	int sockfd,
	const InetAddress& localAddr,
	const InetAddress& peerAddr)
	:loop_(loop),
	name_(name),
	state_(kConnecting),
	socket_(new Socket(sockfd)),
	channel_(new Channel(loop, sockfd)),
	localAddr_(localAddr),
	peerAddr_(peerAddr),
	highWaterMark_(64 * 1024 * 1024), //64M
	reading_(true)
{
	channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
	channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
	channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
	channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
	LOG_DEBUG << "TcpConnection::ctor [" << name_ << "] at " << this << " fd = " << sockfd;
	socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
	LOG_DEBUG << "TcpConnection::dtor[" << name_ << "] at " << this
		<< " fd=" << channel_->fd()
		<< " state=" << stateToString();
	//assert(state_ == kDisconnected);
}

void TcpConnection::send(const void* message, int len)
{
	send(StringPiece(static_cast<const char*>(message), len));
}
void TcpConnection::send(const StringPiece& message)
{
	if (state_ == kConnected)
	{
		if (loop_->isInLoopThread())
		{
			sendInLoop(message);
		}
		else
		{
			loop_->runInLoop(
				std::bind(
					static_cast<void(TcpConnection::*)(const StringPiece&)>(&TcpConnection::sendInLoop), // std::bind overload function 
					this,
					message.as_string())); 
		}
	}
}

void TcpConnection::send(Buffer* buf)
{
	if (state_ == kConnected)
	{
		if (loop_->isInLoopThread())
		{
			sendInLoop(buf->peek(), buf->readableBytes());
		}
		else
		{
			loop_->runInLoop(
				std::bind(static_cast<void(TcpConnection::*)(const StringPiece&)>(&TcpConnection::sendInLoop),
				this,
				buf->retrieveAllAsString()));
		}
	}
}
void TcpConnection::sendInLoop(const StringPiece& message)
{
	sendInLoop(message.data(), message.size());
}
void TcpConnection::sendInLoop(const void* message, size_t len)
{
	loop_->assertInLoopThread();
	ssize_t nwrote = 0;
	ssize_t remaining = len;
	bool faultError = false;
	if (state_ == kDisconnected)
	{
		LOG_WARN << "disconnected,give up writing";
		return;
	}
	// if nothing in output queue,try writing directly
	if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
	{
		nwrote = sockets::write(channel_->fd(), message, len);
		if (nwrote >= 0)
		{
			remaining = len - nwrote;
			if (remaining == 0 && writeCompleteCallback_)
			{
				loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
			}
		}
		else // nwrote <0
		{
			nwrote = 0;
			errno = GetLastError();
			LOG_SYSERR << "TcpConnection::sendInLoop";
			faultError = true;
			//if (errno != EWOULDBLOCK)
			//{
			//	LOG_SYSERR << "TcpConnection::sendInLoop";
			//	if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
			//	{
			//		faultError = true;
			//	}
			//}
		}
	} // end if (!channel_->isWriting()....
	//assert(remaining <= len);
	if (!faultError&&remaining > 0)
	{
		size_t oldLen = outputBuffer_.readableBytes();
		if (oldLen + remaining >= highWaterMark_ &&
			oldLen < highWaterMark_ &&
			highWaterMarkCallback_)
		{
			loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
		}
		outputBuffer_.append(static_cast<const char*>(message) + nwrote, remaining);
		if (channel_->isWriting())
		{
			channel_->enableWriting();
		}
	}
}

void TcpConnection::shutdown()
{
	if (state_ == kConnected)
	{
		setState(kDisconnecting);
		loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
	}
}

void TcpConnection::shutdownInLoop()
{
	loop_->assertInLoopThread();
	// not have writing data in queue
	if (!channel_->isWriting()) 
	{
		socket_->shutdownWrite();
	}
}
void TcpConnection::forceClose()
{
	if (state_ == kConnected || state_ == kDisconnecting)
	{
		setState(kDisconnecting);
		loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
	}
}

void TcpConnection::forceCloseInLoop()
{
	loop_->assertInLoopThread();
	if (state_ == kConnected || state_ == kDisconnecting)
	{
		handleClose();
	}
}
const char* TcpConnection::stateToString()const
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

void TcpConnection::setTcpNoDelay(bool on)
{
	socket_->setTcpNoDelay(on);
}
void TcpConnection::startRead()
{
	loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}
void TcpConnection::startReadInLoop()
{
	loop_->assertInLoopThread();
	if (!reading_ || !channel_->isReading())
	{
		channel_->enableReading();
		reading_ = true;
	}
}
void TcpConnection::stopRead()
{
	loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}
void TcpConnection::stopReadInLoop()
{
	loop_->assertInLoopThread();
	if (reading_ || channel_->isReading())
	{
		channel_->disableReading();
		reading_ = false;
	}
}
void TcpConnection::connectEstablished()
{
	loop_->assertInLoopThread();
	//assert(state_ == kConnecting);
	setState(kConnected);
	channel_->tie(shared_from_this());
	channel_->enableReading();
	connectionCallback_(shared_from_this());
}
void  TcpConnection::connectDestroyed()
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
void TcpConnection::handleRead(Timestamp receiveTime)
{
	loop_->assertInLoopThread();
	int saveErrno = 0;
	ssize_t n = inputBuffer_.readFd(channel_->fd(), &saveErrno);
	if (n > 0)
	{
		messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
	}
	else if(n == 0|| saveErrno == WSAECONNRESET)  //WSAECONNRESET  10054
	{
		handleClose();
	}
	else
	{
		errno = saveErrno;
		LOG_ERROR << "TcpConnection::handleRead";
		handleError();
	}
}
void TcpConnection::handleWrite()
{
	loop_->assertInLoopThread();
	if (channel_->isWriting())
	{
		ssize_t n = sockets::write(channel_->fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
		if (n > 0)
		{
			outputBuffer_.retrieve(n);
			if (outputBuffer_.readableBytes() == 0)
			{
				channel_->disableWriting();
				if (writeCompleteCallback_)
				{
					loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
				}
				if (state_ == kDisconnecting)
				{
					shutdownInLoop();
				}
			}
		}
		else // n<=0
		{
			errno = GetLastError();
			LOG_SYSERR << "TcpConnection::handleWrite";
		}
	}
	else //if (channel_->isWriting())
	{
		LOG_TRACE << "Connection fd = " << channel_->fd() << " is down, no more writing";
	}
}

void TcpConnection::handleClose()
{
	loop_->assertInLoopThread();
	LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToString();
	//assert(state_ == kConnected || state_ == kDisconnecting);
	setState(kDisconnected);
	channel_->disableAll();

	TcpConnectionPtr guardThis(shared_from_this());
	connectionCallback_(guardThis);
	// must be the last line
	closeCallback_(guardThis);
}
void TcpConnection::handleError()
{
	int err = sockets::getSocketError(channel_->fd());
	errno = GetLastError();
	LOG_ERROR << "TcpConnection::handleError [" << name_ << "] -SO_ERROR = " << err << " " << strerror(err);
}
