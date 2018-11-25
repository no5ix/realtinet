#ifndef CALM_BASE_ASYNCLOGGING_H_
#define CALM_BASE_ASYNCLOGGING_H_

#include <count_down_latch.h>
#include <log_stream.h>
#include <uncopyable.h>

#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <memory>

namespace muduo
{
	class AsyncLogging :muduo::uncopyable
	{
	public:
		AsyncLogging(const string& basename,
			size_t rollSize,
			int flushInterval = 3);
		~AsyncLogging()
		{
			if (running_)
			{
				stop();
			}
		}
		void append(const char* logline, int len);
		void start()
		{
			running_ = true;
			th_ = std::thread(std::bind(&AsyncLogging::threadFunc, this));
			latch_.wait();
		}
		void stop()
		{
			running_ = false;
			cond_.notify_one();
			th_.join();
		}
	private:
		void threadFunc();

		typedef muduo::detail::FixedBuffer<muduo::detail::kLargeBuffer> Buffer;
		typedef std::vector<std::shared_ptr<Buffer>> BufferVector;
		typedef std::shared_ptr<Buffer> BufferPtr;

		const int flushInterval_;
		bool running_;
		string basename_;
		size_t rollSize_;
		std::thread th_;
		muduo::CountDownLatch latch_;
		std::mutex mutex_;
		std::condition_variable cond_;
		BufferPtr currentBuffer_;
		BufferPtr nextBuffer_;
		BufferVector buffers_;
	};// end class AsyncLogging
}// end namespace muduo

#endif //CALM_BASE_ASYNCLOGGING_H_
