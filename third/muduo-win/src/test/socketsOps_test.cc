#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")
#include "logging.h"
#include "Endian.h"
#include "SocketsOps.h"

#include <iostream>



using namespace std;
using namespace muduo;
using namespace muduo::net;

//int main()
int main914()
{
	WSADATA  Ws;
	char SendBuffer[MAX_PATH];
	//Init Windows Socket
	 if (WSAStartup(MAKEWORD(2, 2), &Ws) != 0)
	 {
		 cout << "Init Windows Socket Failed::" << GetLastError() << endl;
		 return -1;
	 }
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	//inet_pton(AF_INET,"192.168.255.130",)
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = sockets::hostToNetwork16(2000);
	memset(addr.sin_zero, 0, 8);

	int sockfd = sockets::createOrDie(AF_INET);
	int ret = sockets::connect(sockfd, sockets::sockaddr_cast(&addr));
	if (ret == SOCKET_ERROR)
	{
		LOG_ERROR << "connect error" << GetLastError();
	}
	else
	{
		while (true)
		{
			cin.getline(SendBuffer, sizeof(SendBuffer));
			ret = sockets::write(sockfd, SendBuffer, static_cast<int>(strlen(SendBuffer)));

		}
	}
	cin.get();
	sockets::close(sockfd);
	WSACleanup();
	return 0;
}