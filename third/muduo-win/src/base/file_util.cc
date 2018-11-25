// excerpts from https://github.com/chenshuo/muduo
// modify by:Kevin <gjm_kevin@163.com>
// 2016-08-15 14:10:11 file_util.cc

#define NOMINMAX

#include <file_util.h>
#include <logging.h>

#include <algorithm>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>   
#include <io.h>

using namespace muduo;
#define S_ISREG(m) (((m) & 0170000) == (0100000))
#define S_ISDIR(m) (((m) & 0170000) == (0040000))


FileUtil::AppendFile::AppendFile(StringArg filename)
	:fp_(::fopen(filename.c_str(),"a+")),
	writtenBytes_(0)
{
	assert(fp_);
	::setvbuf(fp_, buffer_, _IOFBF, sizeof(buffer_));
}

FileUtil::AppendFile::~AppendFile()
{
	::fclose(fp_);
}

void FileUtil::AppendFile::append(const char* logline, const size_t len)
{
	size_t n = _write(logline, len);
	size_t remain = len - n;
	while (remain > 0)
	{
		size_t x = _write(logline + n, remain);
		if (x == 0)
		{
			int err = ferror(fp_);
			if (err)
			{
				fprintf(stderr, "AppendFile::append() failed %s\n", strerror(err));
			}
			break;
		}
		n += x;
		remain = len - n;
	}
	writtenBytes_ += len;
}

void FileUtil::AppendFile::flush()
{
	::fflush(fp_);
}
size_t FileUtil::AppendFile::_write(const char* logline, size_t len)
{
	return ::_fwrite_nolock(logline, 1, len, fp_);
}

FileUtil::ReadSmallFile::ReadSmallFile(StringArg filename)
	:fd_(::_open(filename.c_str(),O_RDWR|O_APPEND|O_CREAT)),
	err_(0)
{
	buf_[0] = '\0';
	if (fd_<0)
	{
		err_ = errno;
	}
}
FileUtil::ReadSmallFile::~ReadSmallFile()
{
	if (fd_ > 0)
	{
		::_close(fd_);
	}
}

template<typename String>
int FileUtil::ReadSmallFile::readToString(int maxSize,
		String* content,
		int64_t* fileSize,
		int64_t* modifyTime,
		int64_t* createTime)
{
	assert(content != NULL);
	int err = err_; 
	if (fd_ > 0)
	{
		content->clear();
		if (fileSize)
		{
			struct stat statbuf;
			if (::fstat(fd_, &statbuf) == 0)
			{
				if (S_ISREG(statbuf.st_mode))
				{
					*fileSize = statbuf.st_size;
					content->reserve(static_cast<int>(std::min(implicit_cast<int64_t>(maxSize), *fileSize)));
				}
				else if (S_ISDIR(statbuf.st_mode))
				{
					err = EISDIR;
				}
				if (modifyTime)
				{
					*modifyTime = statbuf.st_mtime;
				}
				if (createTime)
				{
					*createTime = statbuf.st_ctime;
				}
			}
			else
			{
				err = errno;
			}
		}// end if(fileSize)
		while (content->size() < implicit_cast<size_t>(maxSize))
		{
			size_t toRead = std::min(implicit_cast<size_t>(maxSize) - content->size(), sizeof(buf_));
			size_t n = ::_read(fd_,buf_,toRead);
			if (n > 0)
			{
				content->append(buf_, n);
			}
			else
			{
				if (n < 0)
				{
					err = errno;
				}
				break;
			}
		}
	}// end if(fp_ > 0)
	return err;
}

int FileUtil::ReadSmallFile::readToBuffer(int* size)
{
	int err = err_;
	if (fd_ > 0)
	{
		size_t n = ::_read(fd_, buf_, sizeof(buf_) - 1);
		if (n >= 0)
		{
			if (size)
			{
				*size = static_cast<int>(n);
			}
			buf_[n] = '\0';
		}
		else
		{
			err = errno;
		}
	}
	return err;
}
template int FileUtil::readFile(StringArg filename,
	int maxSize,
	std::string* content,
	int64_t*, int64_t*, int64_t*);

template int FileUtil::ReadSmallFile::readToString(
	int maxSize,
	std::string* content,
	int64_t*, int64_t*, int64_t*);

