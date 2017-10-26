/*
 *	@file		MirrorServerInfo.h
 *	@data		2012-12-5 17:43
 *	@author		zhu qing quan
 *	@brief		用于描述MirrorServer简要信息的结构体。
 *				MirrorMonitor模块与MirrorConfigSDK模块使用这个结构体交互MirrorServer的基本信息。
 *  @Copyright	(C) 2012, by Ragile Corporation
 */

#pragma once
#ifndef _SOA_MIRROR_RPC_MIRRORSERVER_INFO_H_
#define _SOA_MIRROR_RPC_MIRRORSERVER_INFO_H_s

#include <vector>
#include <string>
#include <map>
#include "msgpack.hpp"

namespace SOA
{
namespace Mirror
{
namespace RPC
{
	/**
	 * @name	MirrorServerInfo
	 * @brief	描述MirrorServer简要信息，使用msgpack对其序列化与反序列化。通过网络传输用于公布MirrorServer服务器的简要信息。
	 */
	struct MirrorServerInfo_map
	{
		void setProperty(const std::string& propertyName, const std::string& propertyValue);
		void setIPs(const std::vector<std::string>& strIPsVec);
		void setPort(int port);
		void setName(const char* name);
		void setGUID(const char* strGuid);
		void setMACs(const std::vector<std::string>& strMACsVec);

		std::vector<std::string> getLocalIPs() const;
		int getPort()  const;
		std::string getName() const;
		std::string getGUID() const;
		std::vector<std::string> getMACs() const;

		MSGPACK_DEFINE(serverPropertys);
	private:
		typedef std::map<std::string, std::string> PropertyCollection;
		PropertyCollection serverPropertys;
	};

	std::vector<std::string> splitString(const std::string& src, char seq);

	/**
	 * @name	MirrorServerInfo
	 * @brief	描述MirrorServer简要信息，使用msgpack对其序列化与反序列化。通过网络传输用于公布MirrorServer服务器的简要信息。
	 */
	struct MirrorServerInfo
	{
		std::vector<std::string> localIPs;
		int port;
		std::string name;
		std::string strGUID;
		std::string reserving;

		MSGPACK_DEFINE(localIPs, port, name, strGUID, reserving);
	};

	/**
	 * @name	MirrorServerInfo_old
	 * @brief	描述MirrorServer简要信息，使用msgpack对其序列化与反序列化。通过网络传输用于公布MirrorServer服务器的简要信息。
	 *			这个是之前的Monitor版本使用结构体，后来添加了strGUID、reserving两个成员
	 */
	struct MirrorServerInfo_old
	{
		std::vector<std::string> localIPs;
		int port;
		std::string name;

		MSGPACK_DEFINE(localIPs, port, name);
	};
}
}
}

#endif