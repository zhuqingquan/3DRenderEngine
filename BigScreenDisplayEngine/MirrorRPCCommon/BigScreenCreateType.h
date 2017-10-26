/*
 *	@file		BigScreenCreateType.h
 *	@data		9:7:2012   10:50
 *	@author		zhu qing quan	
 *	@brief		定义BigScreen创建类型的的枚举
 *  @Copyright	(C) 2012, by Ragile Corporation
 */

#pragma once
#ifndef _SOA_MIRROR_RPC_BIGSCREEN_CREATE_TYPE_H_
#define _SOA_MIRROR_RPC_BIGSCREEN_CREATE_TYPE_H_

namespace SOA
{
namespace Mirror
{
namespace RPC
{
	typedef enum BigScreenCreateType
	{
		Server_BigScreen = 1,		//创建服务器版本的电视墙的Bigscreen
		WinForm_BigScreen,			//创建WinForm版本的电视墙的Bigscreen
		WPF_Bigscreen,				//创建WPF版本的电视墙的Bigscreen
	};
}
}
}

#endif