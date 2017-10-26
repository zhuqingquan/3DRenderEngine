/*
 * @file	SOANetwork.h
 * @author	zhu qing quan
 * @date	2012-4-16  13:51
 * @brief	SOA项目组公共的网络相关的工具定义实现
			已支持：获取所有IP、子网掩码、广播地址，询问本地IP是否可用
 * @Copyright (C) 2011, by Ragile Corporation
 */

#pragma once

#ifndef __SOANetwork_H__
#define __SOANetwork_H__

#include <string>
#include <vector>

namespace SOA
{
	typedef struct IP_INFO_tag
	{
		std::string ip;
		std::string netMask;
		std::string broadcast;
	}IP_INFO;

	/**
	 * Method		getIPConfig
	 * @brief		获取本机的IP配置，包括IP、子网掩码、广播地址。
	 * @param[out]	ipInfos IP_INFO结构体数组
	 * @return		int 获取到的IP_INFO结构体的个数, -1为获取失败
	 */
	int getIPConfig(std::vector<IP_INFO>& ipInfos);

	/**
	 * Method		isConnectable
	 * @brief		本地IP是否可用
	 * @param[in]	localIP 本地ip地址
	 * @return		true--可用 false--不可用
	 */
	bool isConnectable( const char* localIP );

	/**
	 * Method		isNetworkCableConnected
	 * @brief		判断网线是否连接好
	 * @return		bool true--连接好 false--未连接
	 */
	bool isNetworkCableConnected();

	/**
	 * Method		isHostIPAvailable
	 * @brief		判断主机的IP配置是否可用
	 *				以下两种情况下IP视为异常：
	 *				1、手动配置IP为"169.254.*.*",子网掩码为"255.255.0.0"
	 *				2、没有进行手动配置，且无法从DHCP服务器获取到IP，系统自动将IP设置为"169.254.*.*",子网掩码为"255.255.0.0"
	 * @param[in]	const std::vector<IP_INFO> & ipInfos 本机所有的IP地址
	 * @return		bool true--可用  false--不可用
	 */
	bool isHostIPAvailable(const std::vector<IP_INFO>& ipInfos);
}

#endif // SOANetwork_H__