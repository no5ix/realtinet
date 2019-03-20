

#ifndef UDP_MUDUO_NET_SOCKETSOPS_H
#define UDP_MUDUO_NET_SOCKETSOPS_H

#ifndef _WIN32
#include <arpa/inet.h>
//#else
//#include <windows.h>
//#include <winsock2.h>
//#include <ws2tcpip.h>
#endif


namespace muduo
{
namespace net
{
class Buffer;
namespace sockets
{


#ifndef _WIN32
int createUdpNonblockingOrDie( sa_family_t family );
#else
int createUdpNonblockingOrDie(int family);
#endif

int recvfrom(int sockfd, struct sockaddr_in6* addr, char *packetMem, int packetSize);
}
}
}

#endif  // UDP_MUDUO_NET_SOCKETSOPS_H
