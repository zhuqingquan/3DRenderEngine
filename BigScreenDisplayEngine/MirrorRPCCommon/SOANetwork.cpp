#include "SOANetwork.h"

#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <iostream>

using namespace std;
using namespace SOA;

#pragma comment(lib,"Ws2_32.lib")

int SOA::getIPConfig(std::vector<IP_INFO>& ipInfos)
{
	ipInfos.clear();

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock==INVALID_SOCKET)
	{
		return -1;
	}

	char buffer[10240] = {0};
	int actualBytes = 0;
	if( WSAIoctl(sock, SIO_GET_INTERFACE_LIST, NULL, 0, buffer, 1024, (LPDWORD)&actualBytes, NULL, 0)==SOCKET_ERROR )
	{
		return -1;
	}
	int count = actualBytes / sizeof(INTERFACE_INFO);
	INTERFACE_INFO* pInfo = (INTERFACE_INFO*)buffer;
	for(int i=0; i<count; i++)
	{
		INTERFACE_INFO* aInterface = pInfo+i;
		IP_INFO aInfo;
		char* ip = inet_ntoa(aInterface->iiAddress.AddressIn.sin_addr);
		if(ip==NULL || strlen(ip)<=0 || strcmp(ip, "127.0.0.1")==0 )
			continue;
		aInfo.ip = ip;
		char* mask = inet_ntoa(aInterface->iiNetmask.AddressIn.sin_addr);
		aInfo.netMask = mask;
		char* broadcast = inet_ntoa(aInterface->iiBroadcastAddress.AddressIn.sin_addr);
		aInfo.broadcast = broadcast;
		ipInfos.push_back(aInfo);
	}

	closesocket(sock);
	return ipInfos.size();
}

bool SOA::isConnectable( const char* localIP )
{
	if(localIP==NULL)
		return false;

	std::vector<IP_INFO> ipInfos;
	int nIpInfo;
	if((nIpInfo=SOA::getIPConfig(ipInfos))==-1)
	{
		cout << "Can not get local host IP config" << endl;
		return false;
	}

	std::string strLocalIp = localIP;
	for(int i=0; i<nIpInfo; i++)
	{
		if( strLocalIp==ipInfos[i].ip )
			return true;
	}
	return false;
}

bool SOA::isNetworkCableConnected()
{
	char hostname[1024];
	if( gethostname(hostname,1024)!=0 )
	{
		OutputDebugString("SOA::isNetworkCableConnected : failed in gethostname\n");
		return false;
	}
	hostent *p = gethostbyname(hostname);
	if(p==NULL)
	{
		OutputDebugString("SOA::isNetworkCableConnected : failed in gethostbyname\n");
		return false;
	}
	int count = 0;
	for(int i=0; p->h_addr_list[i]!=NULL; i++)
	{
		++count;
	}
	if(count<=0)
	{
		return false;
	}
	else if(count==1)
	{
		char *p2 = inet_ntoa(*((in_addr *)p->h_addr)); //Get the IpAddress 
		if(strcmp(p2,"127.0.0.1")==0)
			return false;
		else 
			return true;
	}

	return true;
}

bool SOA::isHostIPAvailable(const std::vector<IP_INFO>& ipInfos)
{
	if(ipInfos.size()<=0)
		return false;
	if(ipInfos.size()==1)
	{
		const char* ip = ipInfos[0].ip.c_str();
		const char* mask = ipInfos[0].netMask.c_str();
		if(strncmp(ip, "169.254.", strlen("169.254."))==0 && strcmp(mask, "255.255.0.0")==0)
			return false;
		else
			return true;
	}
	return true;
}