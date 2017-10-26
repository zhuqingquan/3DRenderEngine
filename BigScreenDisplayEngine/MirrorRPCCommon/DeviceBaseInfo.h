/*
 *	@file		DeviceBaseInfo.h
 *	@data		2013-9-17 20:01
 *	@author		zhu qing quan
 *	@brief		设备的基本信息，如设备ID、通道ID、设备类型、型号等
 *  @Copyright	(C) 2013, by Ragile Corporation
 */

#pragma once
#ifndef _SOA_MIRROR_RPC_DEVICE_BASE_INFO_H_
#define _SOA_MIRROR_RPC_DEVICE_BASE_INFO_H_

#include <vector>
#include <string>
#include "msgpack.hpp"

namespace SOA
{
namespace Mirror
{
namespace RPC
{
	/**
	 * @name	DeviceBaseInfo
	 * @brief	设备的基本信息
	 */
	struct DeviceBaseInfo
	{
		std::string deviceID;		//设备ID，用于唯一标识一个设备
		std::string channelID;		//通道ID，如果同一个设备中有多个通道则用于标识通道中的ID
		//std::string url;			//访问设备、获取设备资源、控制设备的URL
		std::string friendlyName;	//友好名称。方便用户读取记忆的别名，可能不唯一
		std::string path;			//从设备驱动中获取的设备所在位置的路径，可以此了解设备的接口信息
		std::string description;	//设备的描述信息，如品牌、生产商、型号等
		std::string category;		//设备类型
		bool		isValid;		//该设备是否可用

		MSGPACK_DEFINE(deviceID, channelID, /*url, */friendlyName, path, description, category, isValid);
	};

	/**
	 * @name	DeviceBaseInfoCollection
	 * @brief	设备基本信息的集合
	 */
	struct DeviceBaseInfoCollection
	{
		std::vector<DeviceBaseInfo> deviceVec;

		MSGPACK_DEFINE(deviceVec);
	};
}
}
}

#endif // _SOA_MIRROR_RPC_DEVICE_BASE_INFO_H_