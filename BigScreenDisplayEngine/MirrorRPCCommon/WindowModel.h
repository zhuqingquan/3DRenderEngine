#pragma once

#include <windows.h>
#include <stdio.h>
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
	HWND	createWindows();
	//void createWindow();
	void showWindow();

	void setWindowRect(RECT rect)
	{
		m_winRect = rect;
	}

	HWND m_hwnd;

private:
	static DWORD getWindowMessageThread(void *pParam);
	void getWindowMessageThreadCallback();
	HWND createWindow(int w_left,int w_top,int w_width,int w_height);
	HWND createWindow_internal(int x,int y,int width,int height,HINSTANCE hInstance);
	RECT m_winRect;
};