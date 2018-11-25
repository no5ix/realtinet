// excerpts from https://github.com/chenshuo/muduo
// modify by:Kevin <gjm_kevin@163.com>
// 2016-08-15 13:47:23 file_util.h

#ifndef CALM_BASE_FILEUTIL_H_
#define CALM_BASE_FILEUTIL_H_

#include <stringpiece.h>
#include <uncopyable.h>
#include <types.h>

namespace muduo
{
namespace FileUtil
{
	//read small file <64k
	class ReadSmallFile
	{
	public:
		ReadSmallFile(StringArg filename);
		~ReadSmallFile();

		template<typename String>
		int readToString(int maxSize,
			String* content,
			int64_t* fileSize,
			int64_t* modifyTime,
			int64_t* createTime);
		//read at maxium kBufferSize into buf_
		int readToBuffer(int* size);
		const char* buffer() const { return buf_; }
		static const int kBufferSize = 64 * 1024;
	private:
		int fd_; // change linux system call open to fopen
		int err_;
		char buf_[kBufferSize];
	};//end class ReadSmallFile
	
	  //read the file content,returns errno if error happens
	 template<typename String>
	 int readFile(StringArg filename,
		int maxSize,
		String* content,
		int64_t* fileSize = NULL,
		int64_t* modifyTime = NULL,
		int64_t* createTime = NULL)
	{
		ReadSmallFile file(filename);
		return file.readToString(maxSize, content, fileSize, modifyTime, createTime);
	}

	// not thread safe
	class AppendFile :muduo::uncopyable
	{
	public:
		explicit AppendFile(StringArg filename);
		~AppendFile();

		void append(const char* logline, const size_t len);
		void flush();
		size_t writtenBytes() const { return writtenBytes_; }
	private:
		size_t _write(const char* logline, size_t len);
		FILE* fp_;
		char buffer_[64 * 1024];
		size_t writtenBytes_;

	};// end class AppendFile
}// end namespace FileUtil

}//end namespace muduo

#endif //CALM_BASE_FILEUTIL_H_
