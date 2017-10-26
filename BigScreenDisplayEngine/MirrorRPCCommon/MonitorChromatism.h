/*
 *	@file		MonitorDisplay.h
 *	@data		2013-8-29 09：41
 *	@author		yao feifei
 *	@brief		色彩控制信息
 *  @Copyright	(C) 2013, by Ragile Corporation
 */

#pragma once
#ifndef _SOA_MIRROR_COMMON_MONITORCHROMATISM_H_
#define _SOA_MIRROR_COMMON_MONITORCHROMATISM_H_

#include "msgpack.hpp"
namespace SOA
{
namespace Mirror
{
namespace Common
{	
struct MonitorChromatism
	{
		int brightness;
		int contrast;
		int rValue;
		int gValue;
		int bValue;
		MSGPACK_DEFINE(brightness,contrast, rValue, gValue, bValue);
	};
}
}
}

#endif // _SOA_MIRROR_COMMON_MONITORCHROMATISM_H_