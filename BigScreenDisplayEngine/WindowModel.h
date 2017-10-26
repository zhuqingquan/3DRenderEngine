#pragma once

#include <windows.h>
#include <stdio.h>
#include "Rectangle.h"
//using namespace std;
class WindowModel
{
private:
	bool	m_bRuning;
	WNDCLASSEX m_wc;
	HANDLE	m_hWindowMessageThread;	
	bool	m_bWindowMessageThreadExit;
	bool	m_bWindowsCreated;
	int	m_monitorCount;
public:
	WindowModel();
	~WindowModel();

	/**
	 * Method		createWinddows
	 * @brief		创建窗体，可以先调用setWindowRect设置窗体所在的位置和大小，再调用该方法创建窗体
	 * @return		HWND 窗体句柄， NULL--失败
	 */
	HWND	createWindows();

	/**
	 * Method		showWindow
	 * @brief		显示窗体
	 * @return		void
	 */
	void showWindow();

	/**
	 * Method		hideWindow
	 * @brief		隐藏窗体
	 * @return		void
	 */
	void hideWindow();

	/**
	 * Method		setBackColor
	 * @brief		设置窗口的背景颜色，没有调用该方法之前窗口为黑色背景
	 * @param[in]	DWORD color 颜色值ARGB
	 * @return		bool true--设置成功  false--设置失败
	 */
	bool setBackColor(DWORD color);

	/**
	 * Method		drawText
	 * @brief		在窗口中的某个区域中显示文字内容，暂时没有提供清除文字内容的接口
	 *				该接口显示简短的内容时效果会较好，因为默认效果是文字内容铺满整个显示区域，不会自动换行
	 * @param[in]	const char * text 文字内容
	 * @param[in]	const SOA::Mirror::RPC::RectCoordinate & rectOfWindow 显示区域，归一化坐标表示相对于整个窗口的坐标
	 * @return		bool true--成功 false--失败
	 */
	bool drawText(const TCHAR* text, const SOA::Mirror::RPC::RectCoordinate& rectOfWindow, COLORREF fontColor);

	/**
	 * Method		setWindowRect
	 * @brief		设置窗口的位置和大小，不要在createWindows调用之前调用，之后调用不会改变窗口位置、大小
	 * @param[in]	RECT rect 窗口的位置和大小
	 * @return		void
	 */
	void setWindowRect(RECT rect)
	{
		m_winRect = rect;
	}

	/**
	 * Method		getWindowRect
	 * @brief		获取窗口的位置和大小
	 * @return		RECT 窗口的位置和大小
	 */
	RECT getWindowRect()
	{
		return m_winRect;
	}

	HWND m_hwnd;

private:
	static DWORD getWindowMessageThread(void *pParam);
	void getWindowMessageThreadCallback();
	HWND createWindow(int w_left,int w_top,int w_width,int w_height);
	HWND createWindow_internal(int x,int y,int width,int height,HINSTANCE hInstance);
	RECT m_winRect;
};