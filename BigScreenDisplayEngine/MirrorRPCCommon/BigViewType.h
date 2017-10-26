/*
 *	@file		BigViewType.h
 *	@data		12:6:2012   15:40
 *	@author		zhu qing quan	
 *	@brief		BigView类型的枚举	
 *  @Copyright	(C) 2012, by Ragile Corporation
 */

#pragma once

#ifndef _BIG_VIEW_TYPE_
#define _BIG_VIEW_TYPE_

namespace SOA
{
namespace Mirror
{
namespace RPC
{
	enum BigViewType
	{
		ImageOnly = 1,	//只画图片,创建在内存中，不能OSD，当图像跨显卡时较快
		OwnerDraw,		//还进行其他的绘画,创建在显存中，可以OSD（通过Texture），当图像跨显卡时比ImageOnly慢一些
	};
}
}
}

#endif