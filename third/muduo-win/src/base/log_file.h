#ifndef CAML_BASE_LOGFILE_H_
#define CAML_BASE_LOGFILE_H_

#include <types.h>
#include <uncopyable.h>
#include <memory>
#include <mutex>

namespace muduo
{

	namespace FileUtil
	{
		class AppendFile;
	}
	class LogFile : muduo::uncopyable
	{
	public:
		LogFile(const string& basename,
			size_t rollSize,
			bool threadSafe = true,
			int flushInterval = 3,
			int checkEveryN = 1024);
		~LogFile();

		void append(const char* logline, int len);
		void flush();
		bool rollFile();
	private:
		void _append_unlock(const char* logline, int len);
		static string _getLogFileName(const string& basename, time_t* now);
		const string basename_;
		const size_t rollSize_;
		bool threadSafe_;
		const int flushInterval_;
		const int checkEveryN_;

		int count_;
		time_t startOfPeriod_;
		time_t lastRoll_;
		time_t lashFlush_;
		mutable  std::mutex mutex_;
		std::shared_ptr<FileUtil::AppendFile> file_;
		const static int kRollPerSeconds = 60 * 60 * 24;
	};// end class LogFile
}// end namespace muduo

#endif CAML_BASE_LOGFILE_H_