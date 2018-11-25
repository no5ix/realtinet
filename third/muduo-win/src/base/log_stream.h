#ifndef CALM_BASE_LOGSTREAM_H_
#define CALM_BASE_LOGSTREAM_H_

#include <types.h>
#include <stringpiece.h>
#include <uncopyable.h>

#include <string.h> 
#include <assert.h>

namespace muduo
{

	namespace detail
	{
		const int kSmallBuffer = 4000;
		const int kLargeBuffer = 4000 * 1000;
		template<int SIZE>
		class FixedBuffer : muduo::uncopyable
		{
		public:
			FixedBuffer()
				:cur_(data_)
			{}
			void append(const char* buf, size_t len)
			{
				if ((implicit_cast<size_t>(avail()) > len))
				{
					memcpy(cur_, buf, len);
					cur_ += len;
				}
			}
			const char* data() const { return data_; }
			int length() const { return static_cast<int>(cur_ - data_); }
			char* current() const { return cur_; }
			int avail() const{ return static_cast<int>(_end() - cur_); }
			void add(size_t len) { cur_ += len; }
			void reset() { cur_ = data_; }
			void bzero()
			{
				memset(data_, 0, sizeof(data_));
				//bzero(data_, sizeof(data_)); 
			}
			std::string toString() const
			{
				return std::string(data_, length());
			}
			StringPiece toStringPiece() const
			{
				return StringPiece(data_, length());
			}
		private:
			const char* _end() const
			{
				return data_ + sizeof(data_);
			}
			void(*cookie)();
			char data_[SIZE];
			char* cur_;
		};//end class FixedBuffer
	} //end namespace detail

	class LogStream : muduo::uncopyable
	{
		typedef LogStream self;
	public:
		typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;
		self& operator<<(bool v)
		{
			buffer_.append(v ? "1" : "0", 1);
			return *this;
		}
		self& operator<<(short);
		self& operator<<(unsigned short);
		self& operator<<(int);
		self& operator<<(unsigned int);
		self& operator<<(long);
		self& operator<<(unsigned long);
		self& operator<<(long long);
		self& operator<<(unsigned long long);
		self& operator<<(const void*);
		self& operator<<(double);
		self& operator<<(float v)
		{
			*this << static_cast<double>(v);
			return *this;
		}
		self& operator<<(char v)
		{
			buffer_.append(&v, 1);
			return *this;
		}
		self& operator<<(const char* str)
		{
			if (str)
			{
				buffer_.append(str, strlen(str));
			}
			else
			{
				buffer_.append("(null)", 6);
			}
			return *this;
		}
		self& operator<<(const unsigned char* str)
		{
			return operator<<(reinterpret_cast<const char*>(str));
		}
		self& operator<<(const std::string& v)
		{
			buffer_.append(v.c_str(), v.length());
			return *this;
		}
		self& operator<<(const StringPiece& v)
		{
			buffer_.append(v.data(), v.size());
			return *this;
		}
		self& operator<<(const Buffer& v)
		{
			*this << v.toString();
			return *this;
		}
		void append(const char* data, int len)
		{
			buffer_.append(data, len);
		}
		const Buffer& buffer() const { return buffer_; }
		void resetBuffer() { buffer_.reset(); }

	private:
		template<typename T>
		void _formatInteger(T);
		Buffer buffer_;
		static const int kMaxNumericSize = 32;
	};// end class LogStream
	class Fmt
	{
	public:
		template<typename T>
		Fmt(const char * fmt, T val);
		const char* data()const{ return buf_; }
		int length() const { return length_; }
	private:
		char buf_[32];
		int length_;
	};// end class Fmt

	inline LogStream& operator<<(LogStream& s, const Fmt& fmt)
	{
		s.append(fmt.data(), fmt.length());
		return s;
	}

}// end namespace muduo

#endif //CALM_BASE_LOGSTREAM_H_
