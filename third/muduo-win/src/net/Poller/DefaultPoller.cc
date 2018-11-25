//////////////////////////////////////////////////////////////////////////
//
// Copyright(C)	2016, gjm_kevin.  All rights reserved.
// Author	:	gjm_kevin@163.com
// Date		:	2016.09.13
//
//////////////////////////////////////////////////////////////////////////

#include "Poller.h"
#include "SelectPoller.h"

#include <stdlib.h>

using namespace muduo::net;

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
	if (1)
	{
		return new SelectPoller(loop);
	}
}