#include "SelectPoller.h"
#include "logging.h"
#include "types.h"
#include "Channel.h"

#include <assert.h>
#include <errno.h>


using namespace muduo;
using namespace muduo::net;

SelectPoller::SelectPoller(EventLoop* loop)
	:Poller(loop)
{}
SelectPoller::~SelectPoller()
{}

Timestamp SelectPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
	// clear all fd_set
	FD_ZERO(&rfds_);
	FD_ZERO(&wfds_);
	FD_ZERO(&efds_);
	unsigned int maxfd = 0; // for select
	for (PollFdList::iterator itor = pollfds_.begin(); itor != pollfds_.end(); ++itor)
	{
		if ((itor->events)&(POLLIN | POLLPRI))
		{
			FD_SET(itor->fd, &rfds_);
		}
		if((itor->events)&(POLLOUT))
		{
			FD_SET(itor->fd, &wfds_);
		}
		FD_SET(itor->fd, &efds_);
		if (itor->fd > maxfd)
			maxfd = itor->fd;
	}
	timeval timeout;
	timeout.tv_sec = timeoutMs/1000;	
	timeout.tv_usec = static_cast<int>(timeoutMs % 1000) * 1000;

	int numEvents = select(maxfd + 1, &rfds_, &wfds_, &efds_, &timeout);
	int saveErrno = GetLastError();

	Timestamp now(Timestamp::now());
	if (numEvents > 0)
	{
		LOG_TRACE << numEvents << " events happened";
		fillActiveChannels(numEvents, activeChannels);
	}
	else if (numEvents == 0)
	{
		LOG_TRACE << "nothing happened";
	}
	else
	{
		if(saveErrno != EINTR)
		{
			errno = saveErrno;
			LOG_SYSERR << "SelectPoller::poll() ";
		}
	}
	return now;	
}

void SelectPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
	for (PollFdList::const_iterator pfd = pollfds_.begin();
		pfd != pollfds_.end() && numEvents > 0; ++pfd)
	{
		int revents = 0;//pfd->revents is readonly
		if (FD_ISSET(pfd->fd, &rfds_))
		{
			revents |= POLLIN;
		}
		if (FD_ISSET(pfd->fd, &wfds_))
		{
			revents |= POLLOUT;
		}
		if (FD_ISSET(pfd->fd, &efds_))
		{
			revents |= POLLERR;
		}
		if (revents > 0)
		{
			--numEvents;
			ChannelMap::const_iterator ch = channels_.find(pfd->fd);
			assert(ch != channels_.end());
			Channel* channel = ch->second;
			assert(channel->fd() == pfd->fd);
			channel->set_revents(revents);
			activeChannels->push_back(channel);
		}

	}
}
void SelectPoller::updateChannel(Channel* channel)
{
	Poller::assertInLoopThread();
	LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
	if (channel->index() < 0)
	{
		assert(channels_.find(channel->fd()) == channels_.end());
		struct pollfd pfd;
		pfd.fd = channel->fd();
		pfd.events = static_cast<short>(channel->events());
		pfd.revents = 0;
		pollfds_.push_back(pfd);
		int idx = static_cast<int>(pollfds_.size()) - 1;
		channel->set_index(idx);
		channels_[pfd.fd] = channel;
	}
	else
	{
		assert(channels_.find(channel->fd()) != channels_.end());
		assert(channels_[channel->fd()] == channel);
		int idx = channel->index();
		assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
		struct pollfd& pfd = pollfds_[idx];
		assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);
		pfd.events = static_cast<short>(channel->events());
		pfd.revents = 0;
		if (channel->isNoneEvent())
		{
			pfd.fd = -channel->fd() - 1;
		}
	}
}

void SelectPoller::removeChannel(Channel* channel)
{
	Poller::assertInLoopThread();
	LOG_TRACE << "fd = " << channel->fd();
	assert(channels_.find(channel->fd()) != channels_.end());
	assert(channels_[channel->fd()] == channel);
	assert(channel->isNoneEvent());
	int idx = channel->index();
	assert(0 <= idx &&idx < static_cast<int>(pollfds_.size()));
	const struct pollfd& pfd = pollfds_[idx];
	(void)pfd;
	assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());
	size_t n = channels_.erase(channel->fd());
	assert(n == 1); (void)n;
	if (implicit_cast<size_t>(idx) == pollfds_.size() - 1)
	{
		pollfds_.pop_back();
	}
	else
	{
		int channelAtEnd = pollfds_.back().fd;
		iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
		if (channelAtEnd < 0)
		{
			channelAtEnd = -channelAtEnd - 1;
		}
		channels_[channelAtEnd]->set_index(idx);
		pollfds_.pop_back();
	}
}