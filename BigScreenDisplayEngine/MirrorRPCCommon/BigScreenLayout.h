/*
 *	@file		BigScreenLayout.h
 *	@data		4:7:2012   16:28
 *	@author		zhu qing quan	
 *	@brief		BigScreen的布局，这个布局表示的是某一台服务器在整个BigScreen中的布局表示
				如当前服务器负责管理BigScreen中第1、3、5个窗格，则BigscreenLayout对象就是这些信息的描述
 *  @Copyright	(C) 2012, by Ragile Corporation
 */

#pragma once
#ifndef _SOA_MIRROR_RPC_BIGSCREEN_LAYOUT_H_
#define _SOA_MIRROR_RPC_BIGSCREEN_LAYOUT_H_

#include "msgpack.hpp"
#include "MirrorTypes.h"
#include "Size.h"

namespace SOA
{
namespace Mirror
{
namespace RPC
{
	/*
	 * 每个窗格的信息
	 */
	struct WindowPane
	{
		WindowPane(float x, float y, int width, int height, PIXFormat format)
			: X(x)
			, Y(y)
			, resolution(width, height)
			, pixelFormat(format)
		{}
		WindowPane()
		{}

		int adapter;
		float X;					//在BigScreen中的左边X轴
		float Y;					//在BigScreen中的左边Y轴
		Size resolution;			//所显示的图像的分辨率
		int pixelFormat;	//显示的图像的像素格式
		MSGPACK_DEFINE(adapter,X, Y, resolution, pixelFormat);
	};

	class BigscreenLayout
	{
	public:
		Size m_size;			//整个Bigscreen的大小，N*M
		std::vector<WindowPane> panes;			//所有窗格信息的列表

		BigscreenLayout(int width, int height)
			: m_size(width, height)
		{}
		BigscreenLayout()
		{}

		void addPane(const WindowPane& pane)
		{
			panes.push_back(pane);
		}
		MSGPACK_DEFINE(m_size, panes);
	};
}
}
}

#endif