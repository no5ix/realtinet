//	20160812 10:23:34
//  kevin <gjm_kevin@163.com>
//  some func for C++11 thread 
#ifndef CALM_BASE_THREAD_H_
#define CALM_BASE_THREAD_H_

#include <types.h>

#include <sstream>
#include <thread>
namespace muduo
{
	inline const uint64_t convertThreadid(std::thread::id pid_t)
	{
		std::stringstream ss;
		ss << pid_t;
		return std::stoull(ss.str());
	}
	inline const uint64_t getCurrentThreadId()
	{
		std::stringstream ss;
		ss << std::this_thread::get_id();
		return std::stoull(ss.str());
	}
}

#endif //CALM_BASE_THREAD_H_
