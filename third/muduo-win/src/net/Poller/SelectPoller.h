//////////////////////////////////////////////////////////////////////////
//
// Copyright(C)	2016, gjm_kevin.  All rights reserved.
// Author	:	gjm_kevin@163.com
// Date		:	2016.09.13
//
//////////////////////////////////////////////////////////////////////////
#ifndef CALM_NET_SELECT_POLLER_H_
#define CALM_NET_SELECT_POLLER_H_
#include "Poller.h"
#include <vector>
struct pollfd;
namespace muduo
{
	namespace net
	{
		class SelectPoller :public Poller
		{
		public:
			SelectPoller(EventLoop* loop);
			virtual ~SelectPoller();

			virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels);
			virtual void updateChannel(Channel* channel);
			virtual void removeChannel(Channel* channel);
		private:
			void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

			typedef std::vector<struct pollfd> PollFdList;
			PollFdList pollfds_;
			fd_set rfds_;
			fd_set wfds_;
			fd_set efds_;
		};// end class SelectPoller
	}// end namespace net
}// end namespace muduo

#endif //CALM_NET_SELECT_POLLER_H_


