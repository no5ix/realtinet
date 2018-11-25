#ifndef CALM_NET_INETADDRESS_H_
#define CALM_NET_INETADDRESS_H_
#include "copyable.h"
#include "stringpiece.h"
#include "SocketsOps.h"

namespace muduo
{
namespace net
{

namespace sockets
{
const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
//const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr) ;
}

class InetAddress :public muduo::copyable
{
public:
	// mostly used in tcp server listening
	// loopbackOnly is refer to 127.0.0.1
	explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false);
	InetAddress(StringArg ip, uint16_t port);

	/// Constructs an endpoint with given struct @c sockaddr_in
	/// Mostly used when accepting new connections
	explicit InetAddress(const struct sockaddr_in& addr)
		: addr_(addr)
	{}

	explicit InetAddress(const struct sockaddr_in6& addr)
		: addr6_(addr)
	{}

	void setSockAddrIn(const struct sockaddr_in& addr) { addr_ = addr; }
	ADDRESS_FAMILY family()const { return addr_.sin_family; }
	string toIp() const;
	string toIpPort() const;
	uint16_t toPort() const;

	const struct sockaddr* getSockAddr() const { return sockets::sockaddr_cast(&addr6_); }

	void setSockAddrInet6(const struct sockaddr_in6& addr6) { addr6_ = addr6; }

	uint32_t ipNetEndian() const;
	uint16_t portNetEndian() const { return addr_.sin_port; };


	// for UdpAcceptor::peerAddrs_
	const struct sockaddr_in6* getSockAddr6() const { return &addr6_; }
	bool operator<(const InetAddress& other) const;


//private:
	//struct sockaddr_in addr_;
private:
	union
	{
		struct sockaddr_in addr_;
		struct sockaddr_in6 addr6_;
	};
};
}//end namespace net
}// end namespace muduo

#endif //CALM_NET_INETADDRESS_H_