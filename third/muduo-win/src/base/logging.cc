#include <logging.h>
#include <timestamp.h>
#include <thread.h>

#include <thread> //thread_local
#include <assert.h>
#include <errno.h> 
#include <stdio.h> // fwrite fflush



namespace muduo
{
	thread_local char t_errnobuf[512];
	thread_local char t_time[32];
	thread_local time_t t_lastSecond;

	//get win environment variable
	//refer to http://stackoverflow.com/questions/4130180/how-to-use-vs-c-getenvironmentvariable-as-cleanly-as-possible
	const char * WinGetEnv(const char * name)
	{
		const int32_t buffSize = 65535;
		static char buffer[buffSize];
		if (GetEnvironmentVariableA(name, buffer, buffSize))
		{
			return buffer;
		}
		else
		{
			return 0;
		}
	}

	const char* strerror_tl(int savedErrno)
	{
		return strerror(savedErrno);
	}

	Logger::LogLevel initLogLevel()
	{
		if (WinGetEnv("CALM_LOG_TRACE"))
		{
			return Logger::LogLevel::TRACE;
		}
		else if (WinGetEnv("CALM_LOG_DEBUG"))
		{
			return Logger::LogLevel::DEBUG;
		}
		else
		{
			return Logger::LogLevel::INFO;
		}
	}

	Logger::LogLevel g_logLevel = initLogLevel();

	// make all the name have same length
	const char* LogLevelName[Logger::LogLevel::NUM_LOG_LEVELS] =
	{
		"TRACE ",
		"DEBUG ",
		"INFO  ",
		"WARN  ",
		"ERROR ",
		"FATAL ",
	};

	//helper class,make param clear and right
	class T
	{
	public:
		T(const char* str,const unsigned len)
			:data_(str),
			len_(len)
		{
			assert(len_ == strlen(data_));
		}
		const char* data_;
		unsigned len_;
	};//end class T

	inline LogStream& operator<<(LogStream& s, T v)
	{
		s.append(v.data_, v.len_);
		return s;
	}
	inline LogStream& operator<<(LogStream& s, Logger::SourceFile& v)
	{
		s.append(v.data_, v.size_);
		return s;
	}
	void defaultOutput(const char*msg, int len)
	{
		size_t n = fwrite(msg, 1, len, stdout);
		(void)n;
	}
	void defaultFlush()
	{
		fflush(stdout);
	}
	Logger::OutputFunc g_output = defaultOutput;
	Logger::FlushFunc g_flush = defaultFlush;
}// end namespace muduo

using namespace muduo;

Logger::Impl::Impl(LogLevel level, int savedErrno, const Logger::SourceFile& file, int line)
	:time_(Timestamp::now()),
	stream_(),
	level_(level),
	line_(line),
	basename_(file)
{
	formatTime();
	stream_ << muduo::getCurrentThreadId() <<" ";
	stream_ << T(LogLevelName[level], 6);
	if(savedErrno != 0)
	{
		//FIXME: for windows GetLastErr() can not get the error message
		stream_ << strerror(savedErrno) << " (errno=" << savedErrno << ") ";
	}
}

void Logger::Impl::formatTime()
{
	int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();
	time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / Timestamp::kMicroSecondsPerSecond);
	int microSeconds = static_cast<int>(microSecondsSinceEpoch % Timestamp::kMicroSecondsPerSecond);
	if (seconds != t_lastSecond)
	{
		t_lastSecond = seconds;
		struct tm tm_time;
		gmtime_s(&tm_time, &seconds);
		int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
			tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
		assert(len == 17);
		(void)len;
	}
	Fmt us(".%06dZ ", microSeconds);
	assert(us.length() == 9);
	stream_ << T(t_time, 17) << T(us.data(), 9);
}

void Logger::Impl::finish()
{
	stream_ << " - " << basename_ << ':' << line_ << '\n';
}


Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
	:impl_(level,0,file,line)
{
	impl_.stream_ << func<<' ';
}
Logger::Logger(SourceFile file, int line, LogLevel level)
	:impl_(level,0,file,line)
{}
Logger::Logger(SourceFile file, int line)
	:impl_(LogLevel::INFO,0,file,line)
{}
Logger::Logger(SourceFile file, int line, bool toAbort)
	: impl_(static_cast<Logger::LogLevel>(toAbort?FATAL:ERR), errno,file,line)
{}
Logger::~Logger()
{
	impl_.finish();
	const LogStream::Buffer& buf(stream().buffer());
	g_output(buf.data(), buf.length());
	if (impl_.level_ == FATAL)
	{
		g_flush();
		abort();
	}
}
void Logger::setLogLevel(Logger::LogLevel level)
{
	g_logLevel = level;
}
void Logger::setOutput(OutputFunc out)
{
	g_output = out;
}
void Logger::setFlush(FlushFunc flush)
{
	g_flush = flush;
}
