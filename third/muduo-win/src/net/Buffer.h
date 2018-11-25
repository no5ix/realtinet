//////////////////////////////////////////////////////////////////////////
//
// Copyright(C)	2016, gjm_kevin.  All rights reserved.
// Author	:	gjm_kevin@163.com
// Date		:	2016.09.09
//
//////////////////////////////////////////////////////////////////////////
#ifndef CALM_NET_BUFFER_H_
#define CALM_NET_BUFFER_H_
//
#pragma warning(disable : 4996)
#include "copyable.h"
#include "stringpiece.h"
#include "types.h"

#include <vector>
//#include <assert.h>
#include <string.h>
#include <algorithm>

#include "Endian.h"

#ifdef _MSC_VER
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

namespace muduo
{
	namespace net
	{
		//////////////////////////////////////////////////////////////////////////
		// excerpts from muduo buffer
		// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
		//
		// @code
		// +-------------------+------------------+------------------+
		// | prependable bytes |  readable bytes  |  writable bytes  |
		// |                   |     (CONTENT)    |                  |
		// +-------------------+------------------+------------------+
		// |                   |                  |                  |
		// 0      <=      readerIndex   <=   writerIndex    <=     size
		// @endcode
		//////////////////////////////////////////////////////////////////////////
		class Buffer:public muduo::copyable
		{
		public:
			static const size_t kCheapPrepend = 8;
			static const size_t kInitialSize = 1024;

			explicit Buffer(size_t initialSize = kInitialSize)
				:buffer_(kCheapPrepend+initialSize),
				readerIndex_(kCheapPrepend),
				writerIndex_(kCheapPrepend)
			{
				
			}

			void swap(Buffer& rhs)
			{
				buffer_.swap(rhs.buffer_);
				std::swap(readerIndex_, rhs.readerIndex_);
				std::swap(writerIndex_, rhs.writerIndex_);
			}
			size_t readableBytes() const
			{
				return writerIndex_ - readerIndex_;
			}
			size_t writableBytes() const
			{
				return buffer_.size() - writerIndex_;
			}
			size_t prependableBytes() const
			{
				return readerIndex_;
			}
			const char* peek() const
			{
				return begin() + readerIndex_;
			}
			const char* findCRLF() const
			{
				const char* crlf = std::search(peek(),beginWrite(), kCRLF, kCRLF + 2);
				return crlf == beginWrite() ? NULL : crlf;
			}
			const char* findCRLF(const char* start) const
			{
				//assert(peek() <= start);
				//assert(start <= beginWrite());
				const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
				return crlf == beginWrite() ? NULL : crlf;
			}
			const char* findEOL() const
			{
				const void* eol = memchr(peek(), '\n', readableBytes());
				return static_cast<const char*>(eol);
			}
			const char* findEOL(const char* start) const
			{
				//assert(peek() <= start);
				//assert(start <= beginWrite());
				const void* eol = memchr(start, '\n', readableBytes());
				return static_cast<const char*>(eol);
			}
			void retrieve(size_t len)
			{
				//assert(len <= readableBytes());
				if (len < readableBytes())
				{
					readerIndex_ += len;
				}
				else
				{
					retrieveAll();
				}
			}
			void retrieveUntil(const char* end)
			{
				//assert(peek() <= end);
				//assert(end < beginWrite());
				retrieve(end - peek());
			}
			void retrieveInt64()
			{
				retrieve(sizeof(int64_t));
			}
			void retrieveInt32()
			{
				retrieve(sizeof(int32_t));
			}
			void retrieveInt16()
			{
				retrieve(sizeof(int16_t));
			}
			void retrieveInt8()
			{
				retrieve(sizeof(int8_t));
			}
			void retrieveAll()
			{
				readerIndex_ = kCheapPrepend;
				writerIndex_ = kCheapPrepend;
			}
			string retrieveAllAsString()
			{
				return retrieveAsString(readableBytes());
			}
			string retrieveAsString(size_t len)
			{
				//assert(len <= readableBytes());
				string result(peek(), len);
				retrieve(len);
				return result;
			}
			StringPiece allToStringPiece()const
			{
				return StringPiece(peek(), static_cast<int>(readableBytes()));
			}
			void append(const StringPiece& str)
			{
				append(str.data(), str.size());
			}
			void append(const char* data, size_t len)
			{
				ensureWritableBytes(len);
				std::copy(data, data + len, beginWrite());
				hasWritten(len);
			}
			void append(const void* data, size_t len)
			{
				append(static_cast<const char*>(data), len);
			}
			void ensureWritableBytes(size_t len)
			{
				if (writerIndex_ < len)
				{
					makeSpace(len);
				}
				//assert(writerIndex_ > len);
			}
			char* beginWrite()
			{
				return begin() + writerIndex_;
			}
			const char* beginWrite() const
			{
				return begin() + writerIndex_;
			}
			void hasWritten(size_t len)
			{
				//assert(len <= writableBytes);
				writerIndex_ += len;
			}
			void unwrite(size_t len)
			{
				//assert(len <= readableBytes);
				writerIndex_ -= len;
			}
			// Append number use network endian
			void appendNetInt64(int64_t x)
			{
				int64_t be64 = sockets::hostToNetwork64(x);
				append(&be64, sizeof(be64));
			}
			void appendNetInt32(int32_t x)
			{
				int32_t be32 = sockets::hostToNetwork32(x);
				append(&be32, sizeof(be32));
			}
			void appendNetInt16(int16_t x)
			{
				int16_t be16 = sockets::hostToNetwork16(x);
				append(&be16, sizeof(be16));
			}
			void appendNetInt8(int8_t x)
			{
				append(&x, sizeof(x));
			}
			///
			/// Read int64_t from network endian
			///
			/// Require: buf->readableBytes() >= sizeof(int32_t)
			int64_t readInt64()
			{
				int64_t result = peekInt64();
				retrieveInt64();
				return result;
			}
			int32_t readInt32()
			{
				int32_t result = peekInt32();
				retrieveInt32();
				return result;
			}
			int16_t readInt16()
			{
				int16_t result = peekInt16();
				retrieveInt16();
				return result;
			}
			int8_t readInt8()
			{
				int8_t result = peekInt8();
				retrieveInt8();
				return result;
			}
			///
			/// Peek int64_t from network endian
			///
			/// Require: buf->readableBytes() >= sizeof(int64_t)
			int64_t peekInt64() const 
			{
				//assert(readableBytes() >= sizeof(int64_t));
				int64_t be64 = 0;
				::memcpy(&be64, peek(), sizeof(be64));
				return sockets::networkToHost64(be64);
			}
			int32_t peekInt32() const
			{
				//assert(readableBytes() >= sizeof(int32_t));
				int32_t be32 = 0;
				::memcpy(&be32, peek(), sizeof(int32_t));
				return sockets::networkToHost32(be32);
			}
			int16_t peekInt16()
			{
				//assert(readableBytes() >= sizeof(int16_t));
				int16_t be16 = 0;
				::memcpy(&be16, peek(), sizeof(int16_t));
				return sockets::networkToHost16(be16);
			}
			int8_t peekInt8()
			{
				//assert(readableBytes() >= sizeof(int8_t));
				int8_t x = *peek();
				return x;
			}
			void prependInt64(int64_t x)
			{
				int64_t be64 = sockets::hostToNetwork64(x);
				prepend(&be64, sizeof(be64));
			}
			void prependInt32(int32_t x)
			{
				int32_t be32 = sockets::hostToNetwork32(x);
				prepend(&be32, sizeof(be32));
			}
			void prependInt16(int16_t x)
			{
				int16_t be16 = sockets::hostToNetwork16(x);
				prepend(&be16, sizeof(be16));
			}
			void prependInt8(int8_t x)
			{
				prepend(&x, sizeof(x));
			}
			void prepend(const void* data, size_t len)
			{
				//assert(len < prependableBytes());
				readerIndex_ -= len;
				const char* d = static_cast<const char*>(data);
				std::copy(d, d + len, begin() + readerIndex_);
			}

			void shrink(size_t reserve)
			{
				Buffer other;
				other.ensureWritableBytes(readableBytes() + reserve);
				other.append(allToStringPiece());
				swap(other);
			}
			size_t internalCapacity() const
			{
				return buffer_.capacity();
			}
			ssize_t readFd(int fd, int* saveErrno);
			
		private:
			char* begin()
			{
				return &*buffer_.begin();
			}
			const char* begin() const
			{
				return &*buffer_.begin();
			}
			void makeSpace(size_t len)
			{
				if (writableBytes() + prependableBytes() < len + kCheapPrepend)
				{
					buffer_.resize(writerIndex_ + len);
				}
				else
				{
					//assert(kCheapPrepend < readableBytes());
					size_t readable = readableBytes();
					std::copy(begin() + readableBytes(),
						begin() + writerIndex_,
						begin() + kCheapPrepend);
					readerIndex_ = kCheapPrepend;
					writerIndex_ = kCheapPrepend + readable;
					//assert(readable == readableBytes());
				}
			}

		private:
			std::vector<char> buffer_;
			size_t readerIndex_;
			size_t writerIndex_;
			static const char kCRLF[];
		};// end class Buffer
	}// end namespace net
}// end namespace muduo

#endif //CALM_NET_BUFFER_H_