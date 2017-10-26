/**
 *	@date		2017:6:22   15:21
 *	@name	 	AdapterOutputHelper.h
 *	@brief		logger for DxRender module
 **/
#ifndef _DX_RENDER_ADAPTER_OUTPUT_HELPER_H_
#define _DX_RENDER_ADAPTER_OUTPUT_HELPER_H_

#include <vector>
#include <Windows.h>
#include "DxZRenderDLLDefine.h"

namespace zRender
{
	/**
	 *	@name			getScreenLogicSysPos
	 *	@brief			获取主机中所有已连接的显示器在操作系统中的逻辑位置坐标.
	 *	@param[out]		std::vector<RECT>& outPosVec
	 *	@return			int outPosVec中信息的个数，-1--失败
	 **/
	int DX_ZRENDER_EXPORT_IMPORT getAllScreenLogicSysPos(std::vector<RECT>& outPosVec);
	/**
	 *	@name			getScreenLogicSysPos
	 *	@brief			获取窗口hwnd所在的显示器在系统中的逻辑位置坐标.
	 *	@param[in]		HWND hwnd
	 *	@return			RECT hwnd所在的显示器在系统中的逻辑位置坐标
	 **/
	RECT DX_ZRENDER_EXPORT_IMPORT getScreenLogicSysPos(HWND hwnd);
}

#endif//_DX_RENDER_ADAPTER_OUTPUT_HELPER_H_