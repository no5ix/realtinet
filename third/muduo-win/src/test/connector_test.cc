//////////////////////////////////////////////////////////////////////////
//
// Copyright(C)	2016, gjm_kevin.  All rights reserved.
// Author	:	gjm_kevin@163.com
//
//////////////////////////////////////////////////////////////////////////

#include "Connector.h"
#include "EventLoop.h"
#include "logging.h"
#include <memory>
#include <iostream>

using namespace muduo;
using namespace muduo::net;

EventLoop * g_loop_connector;

void connectCallback(int sockfd)
{
	LOG_INFO << "connected";
	g_loop_connector->quit();
}

int main_connector()
{
	EventLoop loop;
	g_loop_connector = &loop;

	InetAddress serverAddr("192.168.255.130", 2000);
	std::shared_ptr<Connector> conn(new Connector(&loop, serverAddr));
	conn->setNewConnectionCallback(connectCallback);
	conn->start();

	loop.loop();
	std::cin.get();
	return 0;
}