//////////////////////////////////////////////////////////////////////////
//
// Copyright(C)	2016, gjm_kevin.  All rights reserved.
// Author	:	gjm_kevin@163.com
// Date		:	2016.09.20
//
//////////////////////////////////////////////////////////////////////////
#ifndef CALM_NET_CALLBACKS_H_
#define CALM_NET_CALLBACKS_H_
#include <functional>
#include <memory>
#include "timestamp.h"

namespace muduo
{

	using std::placeholders::_1;
	using std::placeholders::_2;
	using std::placeholders::_3;

	// should really belong to base/Types.h, but <memory> is not included there.

	template<typename T>
	inline T* get_pointer(const std::shared_ptr<T>& ptr)
	{
		return ptr.get();
	}

	template<typename T>
	inline T* get_pointer(const std::unique_ptr<T>& ptr)
	{
		return ptr.get();
	}

	// Adapted from google-protobuf stubs/common.h
	// see License in muduo/base/Types.h
	template<typename To, typename From>
	inline ::std::shared_ptr<To> down_pointer_cast(const ::std::shared_ptr<From>& f)
	{
		if (false)
		{
			implicit_cast<From*, To*>(0);
		}

	#ifndef NDEBUG
		assert(f == NULL || dynamic_cast<To*>(get_pointer(f)) != NULL);
	#endif
		return ::std::static_pointer_cast<To>(f);
	}

	namespace net
	{
		class Buffer;
		class TcpConnection;
		typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
		typedef std::function<void ()> TimerCallback;
		typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
		typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;
		typedef std::function<void(const TcpConnectionPtr&)> WriteCompleteCallback;
		typedef std::function<void(const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;

		typedef std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)>MessageCallback;
		
		void defaultConnectionCallback(const TcpConnectionPtr& conn);
		void defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp receiveTime);
	}// end namespace net
}// end namespace muduo

#endif //CALM_NET_CALLBACKS_H_