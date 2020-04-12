/**
 *	@author		zhuqingquan
 *	@date		2014-10-17
 *	@name		RenderDrawing.h
 *	@brief		the display module of a monitor
 */

#include <Windows.h>
#include <list>

#pragma once
#ifndef _SOA_RENDER_RENDERDRAWING_H_
#define _SOA_RENDER_RENDERDRAWING_H_

#include "BigScreenBackground.h"
#include "ElemDsplModel.h"

namespace zRender
{
	class DisplayElement;
	class DxRender;
	class BasicEffect;
}

namespace SOA
{
namespace Mirror
{
namespace Render
{
	class BigViewportPartition;

	/**
	 *	@name		RenderDrawing
	 *	@brief		BigScreen中一个窗格中所有显示内容的渲染呈现与管理
	 **/
	class RenderDrawing
	{
	public:
		/**
		 *	@name		RenderDrawing
		 *	@brief		构造方法，创建与BigScreen中一个窗格对应的显示模块，该模块显示所有坐标位于该窗格以内的BigViewport的内容
		 *	@param[in]	HWND attatchWnd 窗格中对应的窗口的句柄
		 *	@param[in]	float ltPointX 窗格在整个BigScreen中的坐标，左上顶点的X轴的值
		 *	@param[in]	float ltPointY 窗格在整个BigScreen中的坐标，左上顶点的Y轴的值
		 *	@param[in]	float rbPointX 窗格在整个BigScreen中的坐标，右下顶点的X轴的值
		 *	@param[in]	float rbPointY 窗格在整个BigScreen中的坐标，右下顶点的Y轴的值
		 **/
		RenderDrawing(HWND attatchWnd, float ltPointX, float ltPointY, float rbPointX, float rbPointY, BigScreenBackground* background);

		/**
		 *	@name		~RenderDrawing
		 *	@brief		构造方法，渲染线程将会停止，创建的渲染所用的资源将被释放
		 **/
		~RenderDrawing();

		/**
		 *	@name		start
		 *	@brief		启动渲染线程，渲染所需要的资源将在渲染线程中创建，资源也只能在渲染中使用与释放
		 *	@return		int 0--启动成功	<0--启动失败
		 **/
		int start(HANDLE timerHandle);

		/**
		 *	@name		stop
		 *	@brief		停止渲染线程，渲染线程中创建的资源将被释放
		 *	@return		无
		 **/
		void stop();

		/**
		 *	@name		doRenderWork
		 *	@brief		渲染线程的实际执行的代码，外部用户不能调用该函数
		 **/
		int doRenderWork();

		/**
		 *	@name		addBigViewportPartition
		 *	@brief		添加切分后的BigViewport的部分显示内容，该对象负责渲染所有已添加的BigViewport的显示内容
		 *				BigViewportPartition与DisplayElement对象捆绑，从而外部界面通过BigViewportPartition的接口修改显示的内容
		 *				渲染线程负责释放添加的BigViewportPartition对象在该对象不再需要显示时
		 *	@param[in]	BigViewportPartition* viewportPartition 切分后的BigViewport的部分显示内容
		 *	@return		int	0--添加成功  <0--添加失败
		 **/
		int addBigViewportPartition(BigViewportPartition* viewportPartition);

		/**
		 *	@name		getRegOfBigScreen
		 *	@brief		获取该对象显示渲染的区域在BigScreen中的位置
		 *	@param[out]	float& leftX 窗格在整个BigScreen中的坐标，左上顶点的X轴的值
		 *	@param[out]	float& rightX 窗格在整个BigScreen中的坐标，左上顶点的Y轴的值
		 *	@param[out]	float& topY 窗格在整个BigScreen中的坐标，右下顶点的X轴的值
		 *	@param[out]	float& bottomY 窗格在整个BigScreen中的坐标，右下顶点的Y轴的值
		 *	@return		int 0--成功  <0--失败
		 **/
		int getRegOfBigScreen(float& leftX, float& rightX, float& topY, float& bottomY)
		{
			leftX = m_ltPointX;
			rightX = m_rbPointX;
			topY = m_ltPointY;
			bottomY = m_rbPointY;
			return 0;
		}

		/**
		 *	@name		getWidthInPixel
		 *	@brief		获取该对象显示渲染区域的像素宽度
		 *	@return		int >0--成功 <=0--失败
		 **/
		int getWidthInPixel() const;

		/**
		 *	@name		getHeightInPixel
		 *	@brief		获取该对象显示渲染区域的像素高度
		 *	@return		int >0--成功 <=0--失败
		 **/
		int getHeightInPixel() const;
		//int get

		zRender::DxRender* getDxRender() const { return m_render; }
	private:
		void addViewportPartition(BigViewportPartition* vpPartition);
		//void removeViewportPartition(BigViewportPartition* vpPartition);
		zRender::DisplayElement* createDisplayElement(BigViewportPartition* vpPartition);
		void drawBigViewportPartition(zRender::DxRender* render, BigViewportPartition* vpPartition);

		HWND m_hwnd;
		zRender::DxRender* m_render;
		HANDLE m_timerHandle;

		bool m_isRunning ;
		HANDLE m_thread;
		float m_ltPointX;
		float m_ltPointY;
		float m_rbPointX;
		float m_rbPointY;

		BigScreenBackground* m_background;

		std::list<BigViewportPartition*> m_vpPartitions;
	};
}
}
}

#endif //_SOA_RENDER_RENDERDRAWING_H_