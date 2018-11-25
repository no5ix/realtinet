//////////////////////////////////////////////////////////////////////////
//
// Copyright(C)	2016, gjm_kevin.  All rights reserved.
// Author	:	gjm_kevin@163.com
//
//////////////////////////////////////////////////////////////////////////
#include "Connector.h"
#include "EventLoop.h"
#include "Channel.h"
#include "logging.h"

using namespace muduo;
using namespace muduo::net;

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
	:loop_(loop),
	serverAddr_(serverAddr),
	connect_(false),
	state_(kDisconnected),
	retryDelayMs_(kInitRetryDelayMs)
{
	LOG_DEBUG << "ctor [" << this << "]";
}
Connector::~Connector()
{
	LOG_DEBUG << "dtor ["<<this<<"]";
	assert(!channel_);
}

void Connector::start()
{
	connect_ = true;
	loop_->runInLoop(std::bind(&Connector::startInLoop, this));
	//loop_->queueInLoop(std::bind(&Connector::startInLoop, this));
}
void Connector::startInLoop()
{
	loop_->assertInLoopThread();
	assert(state_ == kDisconnected);
	if (connect_)
	{
		connect();
	}
	else
	{
		LOG_DEBUG << "do not connect";
	}
}
void Connector::stop()
{
	connect_ = false;
	loop_->queueInLoop(std::bind(&Connector::stopInLoop, this));
}
void Connector::stopInLoop()
{
	loop_->assertInLoopThread();
	if (state_ == kConnecting)
	{
		setState(kDisconnected);
		int sockfd = removeAndResetChannel();
		retry(sockfd);
	}
}
void Connector::connect()
{
	int sockfd = sockets::createNoneblockOrDie(serverAddr_.family());
	int ret = sockets::connect(sockfd, serverAddr_.getSockAddr());
	int savedErrno = (0 == ret) ? 0 : GetLastError();
	switch (savedErrno)
	{
	case 0:
	case EINPROGRESS:
	case WSAEINTR:
	case EISCONN:
	case WSAEWOULDBLOCK://WSAEWOULDBLOCK 10035
		connecting(sockfd);
		break;

	case EAGAIN:
	case EADDRINUSE:
	case EADDRNOTAVAIL:
	case ECONNREFUSED:
	case ENETUNREACH:
		retry(sockfd);
		break;

	case EACCES:
	case EPERM:
	case EAFNOSUPPORT:
	case EALREADY:
	case EBADF:
	case EFAULT:
	case ENOTSOCK:
		LOG_SYSERR << "connect error in Connector::startInLoop " << savedErrno;
		sockets::close(sockfd);
		break;

	default:
		LOG_SYSERR << "Unexpected error in Connector::startInLoop " << savedErrno;
		sockets::close(sockfd);
		// connectErrorCallback_();
		break;
	}
}

void Connector::restart()
{
	loop_->assertInLoopThread();
	setState(kDisconnected);
	retryDelayMs_ = kInitRetryDelayMs;
	connect_ = true;
}

void Connector::connecting(int sockfd)
{
	setState(kConnecting);
	assert(!channel_);
	channel_.reset(new Channel(loop_, sockfd));
	channel_->setWriteCallback(std::bind(&Connector::handleWrite, this));
	channel_->setErrorCallback(std::bind(&Connector::handleError, this));

	channel_->enableWriting();
}

int Connector::removeAndResetChannel()
{
	channel_->disableAll();
	channel_->remove();
	int sockfd = channel_->fd();
	loop_->queueInLoop(std::bind(&Connector::resetChannel, this));
	return sockfd;
}
void Connector::resetChannel()
{
	channel_.reset();
}
void Connector::handleWrite()
{
	LOG_TRACE << "Connector::handleWrite " << state_;

	if (state_ == kConnecting)
	{
		int sockfd = removeAndResetChannel();
		int err = sockets::getSocketError(sockfd);
		if (err)
		{
			LOG_WARN << "Connector::handleWrite - SO_ERROR = "
				<< err << " " << strerror(err);
			retry(sockfd);
		}
		else if (sockets::isSelfConnect(sockfd))
		{
			LOG_WARN << "Connector::handleWrite - Self connect";
			retry(sockfd);
		}
		else
		{
			setState(kConnected);
			if (connect_)
			{
				newConnectionCallback_(sockfd);
			}
			else
			{
				sockets::close(sockfd);
			}
		}
	}
	else
	{
		// what happened?
		assert(state_ == kDisconnected);
	}
}

void Connector::handleError()
{
	LOG_ERROR << "Connector::handleError state=" << state_;
	if (state_ == kConnecting)
	{
		int sockfd = removeAndResetChannel();
		int err = sockets::getSocketError(sockfd);
		LOG_TRACE << "SO_ERROR = " << err << " " << strerror(err);
		retry(sockfd);
	}
}
void Connector::retry(int sockfd)
{
	sockets::close(sockfd);
	setState(kDisconnected);
	if (connect_)
	{
		LOG_INFO << "Connector::retry - Retry connecting to " << serverAddr_.toIpPort()
			<< " in " << retryDelayMs_ << " milliseconds. ";
		/*	loop_->runAfter(retryDelayMs_ / 1000.0,
				boost::bind(&Connector::startInLoop, shared_from_this()));
			retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);*/
	}
	else
	{
		LOG_DEBUG << "do not connect";
	}
}
