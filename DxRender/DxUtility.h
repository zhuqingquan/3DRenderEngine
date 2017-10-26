/**
 *	@date		2017:5:24   16:32
 *	@name	 	DxUtility.h
 *	@author		zhuqingquan	
 *	@brief		定义一些全局可用的工具函数
 **/
#pragma once
#ifndef _Z_RENDER_DX_UTILITY_H_
#define _Z_RENDER_DX_UTILITY_H_

#include "DxZRenderDLLDefine.h"

namespace zRender
{
	enum RenderFeatureLevel
	{
		RENDER_FEATURE_LEVEL_UNKNOW = 0, 
		RENDER_FEATURE_LEVEL_DX9_1 = 1,
		RENDER_FEATURE_LEVEL_DX9_2 = 2,
		RENDER_FEATURE_LEVEL_DX9_3 = 3,
		RENDER_FEATURE_LEVEL_DX10_0 = 4,
		RENDER_FEATURE_LEVEL_DX10_1 = 5,
		RENDER_FEATURE_LEVEL_DX11_0 = 6,
		RENDER_FEATURE_LEVEL_DX11_1 = 7,
	};

	DX_ZRENDER_EXPORT_IMPORT RenderFeatureLevel getSupportFeatureLevel();
}

#endif //_Z_RENDER_DX_UTILITY_H_