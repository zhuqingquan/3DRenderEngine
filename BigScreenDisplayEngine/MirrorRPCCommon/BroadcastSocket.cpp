#include "BroadcastSocket.h"

using namespace SOA::Mirror::RPC;


BroadcastSocket::BroadcastSocket( const std::string& multicastIP, int port )
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);

	int iresult = 0;

	m_sendSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if(m_sendSocket==SOCKET_ERROR)
	{
		printf("Error at BroadcastSocket() - Create UDP send socket : %ld\n", WSAGetLastError());
		return;
	}
	int TTL = 5;
	iresult = setsockopt(m_sendSocket, IPPROTO_IP, IP_MULTICAST_TTL,(char*)&TTL, sizeof(TTL));
	if(iresult==SOCKET_ERROR)
	{
		printf("Error at setsockopt(IP_MULTICAST_TTL) : %ld\n", WSAGetLastError());
	}
	int loop = 1;
	iresult = setsockopt(m_sendSocket, IPPROTO_IP, IP_MULTICAST_LOOP,(char*)&loop, sizeof(loop));
	if(iresult==SOCKET_ERROR)
	{
		printf("Error at setsockopt(IP_MULTICAST_TTL) : %ld\n", WSAGetLastError());
	}

	m_recvSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if(m_recvSocket==SOCKET_ERROR)
	{
		printf("Error at BroadcastSocket() - Create UDP recv socket : %ld\n", WSAGetLastError());
		return;
	}

	int on = 1;
	iresult = setsockopt(m_recvSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
	if(iresult==SOCKET_ERROR)
	{
		printf("Error at setsockopt(SO_REUSEADDR) : %ld\n", WSAGetLastError());
	}

	m_multicastAddr.sin_family = AF_INET;
	m_multicastAddr.sin_port = htons(port);
	m_multicastAddr.sin_addr.s_addr = inet_addr(multicastIP.c_str());
	//memcpy(&m_mreq.imr_multiaddr, &m_multicastAddr, sizeof(m_multicastAddr));
	m_mreq.imr_multiaddr.s_addr = inet_addr(multicastIP.c_str());
	m_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	iresult = setsockopt(m_recvSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP,(char*)&m_mreq, sizeof(m_mreq));
	if(iresult==SOCKET_ERROR)
	{
		printf("Error at setsockopt(IP_ADD_MEMBERSHIP) : %ld\n", WSAGetLastError());
	}
	int recloop = 1;
	iresult = setsockopt(m_recvSocket, IPPROTO_IP, IP_MULTICAST_LOOP,(char*)&recloop, sizeof(recloop));
	if(iresult==SOCKET_ERROR)
	{
		printf("Error at setsockopt(IP_MULTICAST_TTL) : %ld\n", WSAGetLastError());
	}
		
	sockaddr_in recvAddr;
	memset(&recvAddr, 0, sizeof(recvAddr));
	recvAddr.sin_family = AF_INET;
	recvAddr.sin_port = htons(port);
	recvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	iresult = bind(m_recvSocket, (SOCKADDR*)&recvAddr, sizeof(recvAddr));
	if(iresult==SOCKET_ERROR)
	{
		printf("ERROR at BroadcastSocket() - bind address : %ld\n", WSAGetLastError());
		return;
	}
}

BroadcastSocket::~BroadcastSocket()
{
	closesocket(m_sendSocket);
	closesocket(m_recvSocket);
	WSACleanup();
}

int BroadcastSocket::send( const char* msg, int offset, int length )
{
	if(m_sendSocket==SOCKET_ERROR)
		return -1;
	int iResult = sendto(m_sendSocket, msg+offset, length, 0, (SOCKADDR *)&m_multicastAddr, sizeof(m_multicastAddr));
	if (iResult == SOCKET_ERROR) 
	{
		printf("Error at BroadcastSocket::send(): %d\n", WSAGetLastError());
		return -1;
	}
	return iResult;
}

int BroadcastSocket::recv( char* msgBuffer, int offset, int length )
{
	fd_set fs;
	FD_ZERO(&fs);
	FD_SET(m_recvSocket, &fs);
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1000*500;
	if (select(m_recvSocket+1, &fs, NULL, NULL, &tv) == -1)
	{
		printf("select() failed with error\n");
		return -1;
	}	
	int iResult = 0;
	if(m_recvSocket>=0 && FD_ISSET(m_recvSocket,&fs))
	{
		sockaddr_in recvFromAddr;
		memset(&recvFromAddr, 0, sizeof(recvFromAddr));
		int recvFromAddrSize = sizeof(recvFromAddr);
		iResult = recvfrom(m_recvSocket, msgBuffer, length-offset, 0, (SOCKADDR *)&recvFromAddr, &recvFromAddrSize);
		if(iResult==SOCKET_ERROR)
		{
			printf("Errot at recv() - recvfrom failed : %ld\n", WSAGetLastError());
			return -1;
		}
	}
	return iResult;
}