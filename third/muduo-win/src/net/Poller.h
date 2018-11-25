#ifndef CALM_NET_POLLER_H_
#define CALM_NET_POLLER_H_

#include <uncopyable.h>
#include <timestamp.h>
#include <EventLoop.h>

#include <map>
#include <vector>

namespace muduo
{
	namespace net
	{
		class Channel;
		class Poller :muduo::uncopyable
		{
		public:
			typedef std::vector<Channel*> ChannelList;

			Poller(EventLoop* loop);
			virtual ~Poller();
			
			virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

			virtual void updateChannel(Channel* channel) = 0;

			virtual void removeChannel(Channel* channel) = 0;
			
			virtual bool hasChannel(Channel* channel) const;

			static Poller* newDefaultPoller(EventLoop* loop);

			void assertInLoopThread() const {
				ownerLoop_->assertInLoopThread();
			}
		protected:
			typedef std::map<int, Channel*> ChannelMap;
			ChannelMap channels_;

		private:
			EventLoop* ownerLoop_;

		};// end class Poller
	}// end namespace net
}// end namespace muduo;

#endif //CALM_NET_POLLER_H_