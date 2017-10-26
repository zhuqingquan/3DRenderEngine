#pragma once

#include <windows.h>
#include "BigScreenSetting.h"
using namespace std;

namespace SOA
{
	namespace Mirror
	{
		namespace RPC
		{

class WindowHandles
{
public:

public:
	WindowHandles();
	WindowHandles(BigScreenSetting bigScreenSetParam);
	~WindowHandles(void);

	int getMonitorCount();
	int getMonitorRect(int index,tagRECT &rect);
	HWND getHandle(int index);
	void showBigScreenSetting();
	void showCursor(bool bShow);
	void showWindow(bool bShow);
	void brindWindowsToTop(bool bTop);

	int	createWindows();
	HMONITOR getMonitor(int index,tagRECT &rect);
	
	/**
	 * Method		setScreenFrequency
	 * @biref		设置显示器的屏幕刷新率
	 * @param[in]	int monitorCount 显示器个数
	 * @param[in]	int frequency 刷新率
	 * @return		int 设置成功返回0，失败返回-1；
	 */
	int setScreenFrequency(int monitorCount,int frequency);
public:
	void monitorEnumProcCallback(HMONITOR hMonitor,  // handle to display monitor
		HDC hdcMonitor,     // handle to monitor DC
		LPRECT lprcMonitor// monitor intersection rectangle
		);
	void getWindowMessageThreadCallback();
	void getWindowCursorThread();
private:
	static BOOL CALLBACK monitorEnumProc(
		HMONITOR hMonitor,  // handle to display monitor
		HDC hdcMonitor,     // handle to monitor DC
		LPRECT lprcMonitor, // monitor intersection rectangle
		LPARAM dwData       // data
		);

	HWND createWindow(int width,int height,HINSTANCE hInstance);
	HWND createWindow(int x,int y,int width,int height,HINSTANCE hInstance);

	void createWindow();
	void createUserWindow();
	static DWORD getWindowMessageThread(void *pParam);
	void initAllMonitorInfo();
private:
	bool cursorStat;
	struct MonitorParam
	{
		MonitorParam()
		{
			hMonitor = NULL;
			hWnd = NULL;
		}
		tagRECT rect;
		HMONITOR hMonitor;
		HWND    hWnd;
	};
	MonitorParam	m_MonitorParam[200];
	bool        	m_bRuning;
	int	m_monitorCount;

	//tagRECT m_ScreenRect[200];

	WNDCLASSEX m_wc;
	HANDLE	m_hWindowMessageThread;
	bool	m_bWindowMessageThreadExit;
	HANDLE  m_hWindowCursorThread;
	bool    m_bWindowCursorThreadExit;
	bool	m_bWindowsCreated;

	BigScreenSetting m_bigScreenSetParam;

	//HWND	*m_phWnd;
};

		}
	}
}