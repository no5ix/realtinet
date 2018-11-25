#include <log_file.h>
#include <file_util.h>
#include <thread.h>
#include <assert.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#ifdef _MSC_VER
#define	snprintf	sprintf_s
#endif


using namespace muduo;

LogFile::LogFile(const string& basename,
	size_t rollSize,
	bool threadSafe /* = true */,
	int flushInterval /* = 3 */,
	int checkEveryN /* = 1024 */)
	:basename_(basename),
	 rollSize_(rollSize),
	threadSafe_(threadSafe),
	flushInterval_(flushInterval),
	checkEveryN_(checkEveryN),
	count_(0),
	startOfPeriod_(0),
	lastRoll_(0),
	lashFlush_(0),
	mutex_()
{
	assert(basename.find('\\') == string::npos);
	rollFile();
}
LogFile::~LogFile()
{}

void LogFile::append(const char* logline, int len)
{
	if (threadSafe_)
	{
		std::unique_lock<std::mutex> lck(mutex_);
		_append_unlock(logline, len);
	}
	else
	{
		_append_unlock(logline, len);
	}
}

void LogFile::flush()
{
	if (threadSafe_)
	{
		std::unique_lock<std::mutex> lck(mutex_);
		file_->flush();
	}
	else
	{
		file_->flush();
	}
}

void LogFile::_append_unlock(const char* logline, int len)
{
	file_->append(logline, len);
	if (file_->writtenBytes() > rollSize_)
	{
		rollFile();
	}
	else
	{
		++count_;
		if(count_ >= checkEveryN_)
		{ 
			count_ = 0;
			time_t now = ::time(NULL);
			// Period只取kRollPerSeconds的整数倍，向下取
			time_t thisPeriod = now / kRollPerSeconds * kRollPerSeconds;
			if (thisPeriod != startOfPeriod_)
			{
				rollFile();
			}
			else if (now - lashFlush_ > flushInterval_)
			{
				lashFlush_ = now;
				file_->flush();
			}
		}
	}
}

bool LogFile::rollFile()
{
	time_t now = 0;
	string filename = _getLogFileName(basename_, &now);
	time_t start = now / kRollPerSeconds*kRollPerSeconds;
	if (now > lashFlush_)
	{
		lastRoll_ = now;
		lashFlush_ = now;
		startOfPeriod_ = start;
		file_.reset(new FileUtil::AppendFile(filename));
		return true;
	}
	return false;
}

string LogFile::_getLogFileName(const string& basename, time_t* now)
{
	string filename;
	filename.reserve(basename.size() + 64);
	filename = basename;
	char timebuf[32];
	struct tm tm_time;
	*now = time(NULL);
	gmtime_s(&tm_time, now);
	strftime(timebuf, sizeof(timebuf),".%Y%m%d-%H%M%S.",&tm_time);
	filename += timebuf;
	filename += "hostname";
	char pidbuf[32];
	snprintf(pidbuf, sizeof(pidbuf), "%" PRIu64 , getCurrentThreadId());
	filename += pidbuf;
	filename += ".log";
	return filename;
}