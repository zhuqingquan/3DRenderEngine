/*
 *	@file		BroadcastSocket.h
 *	@data		2012-12-5 10:35
 *	@author		zhu qing quan
 *	@brief		广播消息发送和接收的SOCKET
 *  @Copyright	(C) 2012, by Ragile Corporation
 */

#pragma once
#ifndef _BROADCAST_SOCKET_H_
#define _BROADCAST_SOCKET_H_

#include <string>
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")

namespace SOA
{
namespace Mirror
{
namespace RPC
{
	/**
	 * @name	BroadcastSocket
	 * @brief	广播消息发送和接收的SOCKET
	 */
	class BroadcastSocket
	{
	public:
		/**
		 * Method		BroadcastSocket
		 * @brief		广播消息的SOCKET，但实际用组播实现
		 * @param[in]	const std::string & multicastIP 组播的IP
		 * @param[in]	int port 组播端口
		 */
		BroadcastSocket(const std::string& multicastIP, int port);
		/**
		 * Method		~BroadcastSocket
		 * @brief		析构函数，释放资源
		 */
		~BroadcastSocket();

		/**
		 * Method		send
		 * @brief		发送消息到组播地址中
		 * @param[in]	const char * msg 消息体
		 * @param[in]	int offset 偏移量
		 * @param[in]	int length 发送的长度
		 * @return		int 返回实际已发送的数据长度，-1为失败
		 */
		int send(const char* msg, int offset, int length);

		/**
		 * Method		recv
		 * @brief		从组播地址中接收数据
		 * @param[in]	char * msgBuffer 存放接收到的数据的内存块
		 * @param[in]	int offset 数据存放的偏移量
		 * @param[in]	int length 可用长度
		 * @return		int 实际接收到的数据的长度， -1为失败
		 */
		int recv(char* msgBuffer, int offset, int length);

	private:
		SOCKET m_sendSocket;
		SOCKET m_recvSocket;
		sockaddr_in m_multicastAddr;
		struct ip_mreq m_mreq;
	};
}
}
}

#endif