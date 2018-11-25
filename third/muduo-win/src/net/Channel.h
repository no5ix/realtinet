#ifndef CALM_NET_CHANNEL_H_
#define CALM_NET_CHANNEL_H_

#include <uncopyable.h>
#include <timestamp.h>

#include <functional>
#include <memory>
namespace muduo
{
	namespace net
	{
		class EventLoop;

		class Channel :muduo::uncopyable
		{
		public:
			typedef std::function<void()> EventCallback;
			typedef std::function<void(Timestamp)> ReadEventCallback;

			Channel(EventLoop* loop, int fd);
			~Channel();
			void handleEvent(Timestamp receiveTime);
			void setReadCallback(const ReadEventCallback& cb)
			{
				readCallback_ = cb;
			}
			void setWriteCallback(const EventCallback& cb)
			{
				writeCallback_ = cb;
			}
			void setCloseCallback(const EventCallback& cb)
			{
				closeCallback_ = cb;
			}
			void setErrorCallback(const EventCallback& cb)
			{
				errorCallback_ = cb;
			}
			void tie(const std::shared_ptr<void>&);
			int fd() const { return fd_; }
			int events() const { return events_; }
			void set_revents(int revt) { revents_ = revt; }
			bool isNoneEvent() const { return events_ == kNoneEvent; }

			void enableReading()
			{
				events_ |= kReadEvent;
				update();
			}
			void disableReading()
			{
				events_ &= ~kReadEvent;
				update();
			}
			void enableWriting()
			{
				events_ |= kWriteEvent;
				update();
			}
			void disableWriting()
			{
				events_ &= ~kWriteEvent;
				update();
			}
			void disableAll()
			{
				events_ = kNoneEvent;
				update();
			}

			bool isWriting() const
			{
				return (events_ & kWriteEvent) != 0;
			}
			bool isReading() const
			{
				return (events_ & kReadEvent) != 0;
			}
			int index() { return index_; }
			void set_index(int idx) { index_ = idx; }

			std::string reventsToString()const;
			std::string eventsToString()const;

			void doNotLogHup()  { logHup_ = false; };
			EventLoop* ownerLoop() { return loop_; }
			void remove();

		private:
			static string eventsToString(int fd, int ev);
			void update();
			void handleEventWithGuard(Timestamp receiveTime);
			static const int kNoneEvent;
			static const int kReadEvent;
			static const int kWriteEvent;

			int index_;
			const int fd_;
			int events_;
			int revents_;
			int logHup_;

			std::weak_ptr<void> tie_;
			bool tied_;
			bool eventHandling_;
			bool addedToLoop_;

			EventLoop* loop_;
			ReadEventCallback readCallback_;
			EventCallback writeCallback_;
			EventCallback closeCallback_;
			EventCallback errorCallback_;
			
			

		};
	}// end namespace net
}// end namespace muduo

#endif //CALM_NET_CHANNEL_H_