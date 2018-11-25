#include "InetAddress.h"
#include "logging.h"
#include "Endian.h"

//#include "types.h"

using namespace muduo;
using namespace muduo::net;

InetAddress::InetAddress(uint16_t port /* = 0 */, bool loopbackOnly /* = false */)
{
	memset(&addr_, 0, sizeof(addr_));
	addr_.sin_family = AF_INET;
	if (loopbackOnly)
	{
		addr_.sin_addr.s_addr = sockets::hostToNetwork32(INADDR_LOOPBACK);
	}
	else
	{
		addr_.sin_addr.s_addr = sockets::hostToNetwork32(INADDR_ANY);
	}
	addr_.sin_port = sockets::hostToNetwork16(port);
}

InetAddress::InetAddress(StringArg ip, uint16_t port)
{
	memset(&addr_, 0, sizeof(addr_));
	sockets::fromIpPort(ip.c_str(), port, &addr_);
}

string InetAddress::toIp() const
{
	char buf[64] = { 0 };
	sockets::toIp(buf, sizeof(buf), getSockAddr());
	return buf;
}

uint16_t InetAddress::toPort() const
{
	return sockets::networkToHost16(addr_.sin_port);
}

string InetAddress::toIpPort() const
{
	char buf[64] = { 0 };
	sockets::toIpPort(buf, sizeof(buf), getSockAddr());
	return buf;
}
uint32_t InetAddress::ipNetEndian() const
{
	return addr_.sin_addr.s_addr;
}

bool InetAddress::operator<(const InetAddress& other) const
{
	const struct sockaddr_in6* thisAddr = this->getSockAddr6();
	const struct sockaddr_in6* otherAddr = other.getSockAddr6();
	if (thisAddr->sin6_family == AF_INET)
	{
		const struct sockaddr_in* laddr4 = reinterpret_cast<const struct sockaddr_in*>(thisAddr);
		const struct sockaddr_in* raddr4 = reinterpret_cast<const struct sockaddr_in*>(otherAddr);
		if (laddr4->sin_addr.s_addr < raddr4->sin_addr.s_addr)
		{
			return true;
		}
		else if (laddr4->sin_port < raddr4->sin_port)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (thisAddr->sin6_family == AF_INET6)
	{
		if (memcmp(&thisAddr->sin6_addr, &otherAddr->sin6_addr, sizeof thisAddr->sin6_addr) < 0)
		{
			return true;
		}
		else if (thisAddr->sin6_port < otherAddr->sin6_port)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}