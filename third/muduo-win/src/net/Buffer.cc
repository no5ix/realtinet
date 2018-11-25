#include "Buffer.h"
#include "SocketsOps.h"

using namespace muduo;
using namespace muduo::net;

const char Buffer::kCRLF[] = "\r\n";

ssize_t muduo::net::Buffer::readFd(int fd, int * saveErrno)
{
	char extrabuf[65536];
	IOV_TYPE vec[2];
	const size_t writable = writableBytes();
	vec[0].buf = begin() + writerIndex_;
	vec[0].len = writable;
	vec[1].buf = extrabuf;
	vec[1].len = sizeof(extrabuf);
	const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
	const ssize_t n = sockets::readv(fd, vec, iovcnt);
	if (n < 0)
	{
		*saveErrno = GetLastError();
	}
	else if(implicit_cast<size_t>(n) <= writable)
	{
		writerIndex_ += n;
	}
	else
	{
		writerIndex_ = buffer_.size();
		append(extrabuf, n - writable);
	}
	return  n;
}
