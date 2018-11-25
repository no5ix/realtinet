#ifndef CALM_NET_ENDIAN_H_
#define CALM_NET_ENDIAN_H_

#include <stdint.h>
#pragma comment( lib, "ws2_32.lib"  )  // for select 
#pragma comment(lib,"ws2_32")
#include <WinSock2.h>


//#ifndef ULONG64
//#define unsigned long long ULONG64
//#endif



namespace muduo
{
	namespace net
	{
		namespace sockets
		{
			inline uint64_t hostToNetwork64(uint64_t host64)
			{
				uint64_t   ret = 0;
				uint32_t   high, low;

				low = host64 & 0xFFFFFFFF;
				high = (host64 >> 32) & 0xFFFFFFFF;
				low = htonl(low);
				high = htonl(high);
				ret = low;
				ret <<= 32;
				ret |= high;
				return   ret;
			}
			inline uint32_t hostToNetwork32(uint32_t host32)
			{
				return htonl(host32);
			}
			inline uint16_t hostToNetwork16(uint16_t host16)
			{
				return htons(host16);
			}
			inline uint64_t networkToHost64(uint64_t net64)
			{
				uint64_t   ret = 0;
				uint32_t   high, low;

				low = net64 & 0xFFFFFFFF;
				high = (net64 >> 32) & 0xFFFFFFFF;
				low = ntohl(low);
				high = ntohl(high);

				ret = low;
				ret <<= 32;
				ret |= high;
				return   ret;
			}
			inline uint32_t networkToHost32(uint32_t net32)
			{
				return ntohl(net32);
			}
			inline uint16_t networkToHost16(uint16_t net16)
			{
				return ntohs(net16);
			}
		}//end namespace sockets
	}//end namespace net
}// end namespace muduo

#endif //CALM_NET_ENDIAN_H_