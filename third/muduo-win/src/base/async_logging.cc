#include <async_logging.h>
#include <log_file.h>
#include <timestamp.h>

#include <stdio.h>

#ifdef _MSC_VER
#define snprintf	sprintf_s
#endif

using namespace muduo;

AsyncLogging::AsyncLogging(const string& basename,
	size_t rollSize,
	int flushInterval /* = 3 */)
	:flushInterval_(flushInterval),
	running_(false),
	basename_(basename),
	rollSize_(rollSize),
	th_(),
	latch_(1),
	mutex_(),
	cond_(),
	currentBuffer_(new Buffer),
	nextBuffer_(new Buffer),
	buffers_()
{
	currentBuffer_->bzero();
	nextBuffer_->bzero();
	buffers_.reserve(16);
}

void AsyncLogging::append(const char* logline, int len)
{
	std::unique_lock<std::mutex> lck(mutex_);
	if (currentBuffer_->avail() > len)
	{
		currentBuffer_->append(logline, len);
	}
	else
	{
		buffers_.push_back(currentBuffer_);
		if (nextBuffer_)
		{
			currentBuffer_ = nextBuffer_;
		}
		else
		{
			currentBuffer_.reset(new Buffer);
		}
		currentBuffer_->append(logline, len);
		cond_.notify_one();
	}// end if (currentBuffer_->avail() > len)
}

void AsyncLogging::threadFunc()
{
	assert(running_ == true);
	latch_.countDown();
	LogFile output(basename_, rollSize_, false);
	BufferPtr newBuffer1(new Buffer);
	BufferPtr newBuffer2(new Buffer);
	newBuffer1->bzero();
	newBuffer2->bzero();
	BufferVector bufferToWrite;
	bufferToWrite.reserve(16);
	while (running_)
	{
		assert(newBuffer1 && newBuffer1->length() == 0);
		assert(newBuffer2 && newBuffer2->length() == 0);
		assert(bufferToWrite.empty());
		{
			std::unique_lock<std::mutex> lck(mutex_);
			if (buffers_.empty()) // unusual usage!
			{
				cond_.wait_for(lck, std::chrono::seconds(flushInterval_));
			}
			buffers_.push_back(currentBuffer_);
			currentBuffer_ = std::move(newBuffer1);
			bufferToWrite.swap(buffers_);
			if (!nextBuffer_)
			{
				nextBuffer_ = std::move(newBuffer2);
			}
		}// end lck
		assert(!bufferToWrite.empty());
		if (bufferToWrite.size() > 25)
		{
			char buf[256];
			snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
				Timestamp::now().toFormattedString().c_str(),
				bufferToWrite.size() - 2);
			fputs(buf, stderr);
			output.append(buf, static_cast<int>(strlen(buf)));
			bufferToWrite.erase(bufferToWrite.begin() + 2, bufferToWrite.end());
		}
		for (size_t i = 0; i < bufferToWrite.size(); ++i)
		{
			output.append(bufferToWrite[i]->data(), bufferToWrite[i]->length());
		}
		if (bufferToWrite.size() > 2)
		{
			bufferToWrite.resize(2);
		}
		if (!newBuffer1)
		{
			assert(!bufferToWrite.empty());
			newBuffer1 = bufferToWrite.back();
			bufferToWrite.pop_back();
			newBuffer1->reset();
		}

		if (!newBuffer2)
		{
			assert(!bufferToWrite.empty());
			newBuffer2 = bufferToWrite.back();
			bufferToWrite.pop_back();
			newBuffer2->reset();
		}
		bufferToWrite.clear();
		output.flush();
	}// end while (running_)
	output.flush();
}