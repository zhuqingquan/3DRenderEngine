/*
 *	@file		Rectangle.h
 *	@data		13:6:2012   10:08
 *	@author		zhu qing quan	
 *	@brief		矩形区域的表示	
 *  @Copyright	(C) 2012, by Ragile Corporation
 */
#pragma once

#ifndef _RECTANGLE_SOA_MIRROR_RPC_
#define _RECTANGLE_SOA_MIRROR_RPC_

//#include "msgpack.hpp"

namespace SOA
{
namespace Mirror
{
namespace RPC
{
struct RectCoordinate
{
	float left;
	float top;
	float right;
	float bottom;

	float getWidth()
	{
		return right - left;
	}
	float getHeight()
	{
		return bottom - top;
	}

	/*
	 * 判断一个区域是否正确，此区域可以出现负数。
	 */
	static int justRectParam(const RectCoordinate &rect)
	{
		if(rect.left >= rect.right ||
			rect.top >= rect.bottom)
		{
			return -1;
		}
		return 0;
	}

	/*
	 * 判断一个区域是否正确，并且此区域为一个正的区域，不会出现负数。
	 */
	static int justRectParamByPositive(const RectCoordinate &rect)
	{
		if(rect.left < 0 || rect.top < 0)
			return -1;
		if(rect.left >= rect.right ||
		rect.bottom <= rect.top)
		{
			return -2;
		}
		return 0;
	}

	/*
	 * 判断一个区域是否正确，并且是归一化后的区域。
	 */
	static int justRectParamByNormalized(const RectCoordinate &rect)
	{
		if( rect.left < 0 || rect.top < 0)
		{
			return -1;
		}
		if( 1 < rect.right || 1 < rect.bottom)
		{
			return -2;
		}
		if(rect.left >= rect.right ||
		rect.bottom <= rect.top)
		{
			return -3;
		}
		return 0;
	}

	//MSGPACK_DEFINE(left, top, right, bottom);
};
}
}
}
#endif