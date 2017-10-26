/*
 *	@file		MonitorDisplay.h
 *	@data		2013-6-21 14:28
 *	@author		zhu qing quan
 *	@brief		获取所有显示器的信息以及附加的一些与之相关的操作
 *  @Copyright	(C) 2013, by Ragile Corporation
 */

#pragma once
#ifndef _SOA_MIRROR_COMMON_MONITORDISPLAY_H_
#define _SOA_MIRROR_COMMON_MONITORDISPLAY_H_

#include <Windows.h>

namespace SOA
{
namespace Mirror
{
namespace Common
{
	struct MonitorDisplayInfo
	{
		LONGLONG id;				//显示器ID
		RECT rect;				//显示器桌面在整个系统虚拟桌面中的位置
		HMONITOR handle;			//用于操作显示器的句柄
		int refreshRate;			//显示器的刷新率

		MonitorDisplayInfo()
			: id(0)
			, handle(NULL)
			, refreshRate(0)
		{

		}
	};

	struct MonitorDisplayInfoList
	{
		int count;					//总数
		char* pBuf;					//MonitorDisplayInfo数组的首地址

		MonitorDisplayInfoList ()
		:count(0),pBuf(NULL)
		{};
	};

	/**
	 * Method		getAllMonitorDispalyInfo
	 * @brief		获取当前所有可用的显示器信息
	 * @param[in]	MonitorDisplayInfoList & infoList 所有可用显示器的信息
	 *				调用该函数时用户初始化infoList，如：
	 *				MonitorDispalyInfoList infoList;
	 *				infoList.count = 100; //显示器个数，给个大点的值
	 *				infoList.pBuf = (char*)malloc(100*sizeof(MonitorDisplayInfo));
	 *				函数调用成功，则修改infoList.count的值为实际有效的显示器的信息
	 *				infoList.pBuf指向显示器信息的数组的首地址
	 *				用户负责释放infoList.pBuf指向的内存
	 * @return		int 0--成功  <0--失败
	 */
	int getAllMonitorDispalyInfo(MonitorDisplayInfoList& infoList);

	/**
	 * Method		compareMonitorDisplayInfoList
	 * @brief		比较两个MonitorDisplayInfoList的内容是否相同
	 * @param[in]	const MonitorDisplayInfoList & left 等号左边
	 * @param[in]	const MonitorDisplayInfoList & right 等号右边
	 * @return		int 0--相同  其他值--不同
	 */
	int compareMonitorDisplayInfoList(const MonitorDisplayInfoList& left, const MonitorDisplayInfoList& right);

	/**
	 * Method		extendDestopToMonitorDispaly
	 * @brief		将系统桌面拓展到所有的显示器上
	 * @return		int 0--成功  <0--失败
	 */
	int extendDestopToMonitorDispaly();
}
}
}

#endif // _SOA_MIRROR_COMMON_MONITORDISPLAY_H_