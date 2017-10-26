#include "WindowHandles.h"
#include <stdio.h>
#include "MonitorDisplayInfo.h"
//#include "DxRender.h"

using namespace SOA;
using namespace Mirror;
using namespace RPC;
using namespace SOA::Mirror::Common;
//using namespace Render;

HICON g_hIcon = NULL;
LRESULT WINAPI WindowHandlesMsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if(WM_CLOSE == msg || WM_QUIT == msg)
	{
		::PostMessage(hWnd,WM_QUIT,NULL,NULL);
		return 0;
	}
	if(WM_PAINT == msg)
	{
		ValidateRect(hWnd, NULL);
		return 0;
	}
	if(WM_SIZE == msg || WM_GETMINMAXINFO == msg)
		return 0;
	//if(WM_DESTROY == msg)
	//	return 0;
	if(WM_GETICON == msg)
	{
		return 0;
		if(ICON_BIG == wParam)
		{
			printf("-----get big icon-----\n");
			return (LRESULT)g_hIcon;
		}
		else if(ICON_SMALL == wParam)
		{
			printf("-----get small icon-----\n");
			return (LRESULT)g_hIcon;
		}
	}
	//printf("-----%x-----\n",msg);
	return DefWindowProc(hWnd, msg, wParam, lParam);
}


SOA::Mirror::RPC::WindowHandles::WindowHandles()
:m_monitorCount(0)
,m_bWindowCursorThreadExit(true)
,m_bWindowMessageThreadExit(true)
,m_bWindowsCreated(false)
,m_hWindowMessageThread(NULL)
,m_hWindowCursorThread(NULL)
,cursorStat(true)
{
	//EnumDisplayMonitors(NULL, NULL, monitorEnumProc, (LPARAM)this);
	initAllMonitorInfo();
}
int getCursorSleepTime(void *para)
{
	LASTINPUTINFO lastInputInfo;
	lastInputInfo.cbSize=sizeof(LASTINPUTINFO);
	if(GetLastInputInfo(&lastInputInfo))
	{
		int result=(GetTickCount()-lastInputInfo.dwTime)/1000;
		return result;
	}
	return 0;

}

int controlCursor(void *para)
{
	WindowHandles *pThis = (WindowHandles*)para;
	if(NULL!=pThis)
	{
		pThis->getWindowCursorThread();
	}
	return 0;
}
WindowHandles::WindowHandles(BigScreenSetting bigScreenSetParam) : m_monitorCount(0)
				//,m_phWnd(NULL)
				,m_bWindowMessageThreadExit(true)
				,m_bWindowCursorThreadExit(true)
				,m_bWindowsCreated(false)
				,m_hWindowMessageThread(NULL)
				,m_hWindowCursorThread(NULL)
				,cursorStat(true)

{
	m_bigScreenSetParam.ID = bigScreenSetParam.ID;
	m_bigScreenSetParam.width = bigScreenSetParam.width;
	m_bigScreenSetParam.height = bigScreenSetParam.height;
	m_bigScreenSetParam.localIP = bigScreenSetParam.localIP;
	m_bigScreenSetParam.localPort = bigScreenSetParam.localPort;
	m_bigScreenSetParam.modeType = bigScreenSetParam.modeType;
	vector<BigScreenMonitor>::iterator iter;
	for(iter = bigScreenSetParam.monitors.begin(); iter != bigScreenSetParam.monitors.end(); ++iter )
	{
		m_bigScreenSetParam.monitors.push_back(*iter);
	}
	//EnumDisplayMonitors(NULL, NULL, monitorEnumProc, (LPARAM)this);
	initAllMonitorInfo();
	m_bRuning = true;
	
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WindowHandlesMsgProc, 0L, 0L,//PRF_NONCLIENT//CS_CLASSDC
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "WindowHandles", NULL };
	g_hIcon = wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	memcpy(&m_wc,&wc,sizeof(WNDCLASSEX));

	m_wc.hbrBackground = (HBRUSH)::GetStockObject(BLACK_BRUSH);//COLOR_BACKGROUND


}

WindowHandles::~WindowHandles(void)
{
	m_bRuning = false;
	for(int i=0; i<m_monitorCount; i++)
	{
		if(NULL != m_MonitorParam[i].hWnd)
		{
			::PostMessage(m_MonitorParam[i].hWnd,WM_QUIT,NULL,NULL);
			::CloseWindow(m_MonitorParam[i].hWnd);
			::DestroyWindow(m_MonitorParam[i].hWnd);
		}
	}

	UnregisterClass("WindowHandles",m_wc.hInstance);

	while(false == m_bWindowMessageThreadExit)
	{
		Sleep(1);
	}
	if(NULL!=m_hWindowMessageThread)
	{
		CloseHandle(m_hWindowMessageThread);
	}
	while(false==m_bWindowCursorThreadExit)
	{
		Sleep(1);
	}
	if(NULL!=m_hWindowCursorThread)
	{
		CloseHandle(m_hWindowCursorThread);
	}

	//delete m_phWnd;
	::ShowCursor(true);
}

int WindowHandles::getMonitorCount()
{
	return m_monitorCount;
}

int WindowHandles::getMonitorRect(int index,tagRECT &rect)
{
	memset(&rect,0,sizeof(tagRECT));
	if(index >= m_monitorCount)
		return 0;
	rect = m_MonitorParam[index].rect;
	return 0;
}


HWND WindowHandles::createWindow(int width,int height,HINSTANCE hInstance)
{
	//HWND hWnd = CreateWindow("WindowHandles", "WindowHandles", WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_POPUP, //WS_OVERLAPPEDWINDOW//WS_POPUPWINDOW
	//	0, 0, width, height, NULL, NULL, hInstance, NULL);
	HWND hWnd = CreateWindow("WindowHandles", "WindowHandles", WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_POPUP, //WS_OVERLAPPEDWINDOW//WS_POPUPWINDOW
		0, 0, width, height, NULL, NULL, hInstance, NULL);
	return hWnd;
}

HWND WindowHandles::getHandle(int index)
{
	HWND hWnd = NULL;
	if(index >= m_monitorCount)
		return NULL;
	return m_MonitorParam[index].hWnd;
}

void WindowHandles::createWindow()
{
	
	if(0 >= m_monitorCount)
		return;
	//m_phWnd = new HWND[m_monitorCount];
	//memset(m_phWnd,0,sizeof(HWND)*m_monitorCount);
	int width = 0;
	int height = 0;
	HDC hDC = NULL;
	tagRECT rect;

	RegisterClassEx(&m_wc);
	char buffer[512];
	int top_offset = 0;
	int left_offset = 0;
	int line_height = 0;
	int i = 0;
	for(vector<BigScreenMonitor>::iterator iter = m_bigScreenSetParam.monitors.begin();
		iter !=  m_bigScreenSetParam.monitors.end();
		++iter)
	{
		if(iter->index >= m_monitorCount)
			continue;
		i = iter->index;
		//width = m_ScreenRect[i].right - m_ScreenRect[i].left;
		//height = m_ScreenRect[i].bottom - m_ScreenRect[i].top;
		width = m_MonitorParam[i].rect.right - m_MonitorParam[i].rect.left;
		height = m_MonitorParam[i].rect.bottom - m_MonitorParam[i].rect.top;
		top_offset = height / 8;
		left_offset = width / 16;
		line_height = (height - top_offset * 2) / 4;
		//m_phWnd[i] = createWindow(width,height,m_wc.hInstance);
		m_MonitorParam[i].hWnd = createWindow(m_MonitorParam[i].rect.left,m_MonitorParam[i].rect.top,width,height,m_wc.hInstance);
		if(NULL == m_MonitorParam[i].hWnd)
		{ 
			printf("create window failed! index = %d\n",i);
			continue;
		}

		//::MoveWindow(m_phWnd[i],m_ScreenRect[i].left,m_ScreenRect[i].top,width,height,false);
		//::SetWindowPos(m_phWnd[i],HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
		//::ShowWindow(m_phWnd[i],SW_SHOW);
	}
	m_bWindowsCreated = true;
}

void SOA::Mirror::RPC::WindowHandles::createUserWindow()
{
	if(0 >= m_monitorCount)
		return;
	//m_phWnd = new HWND[m_monitorCount];
	//memset(m_phWnd,0,sizeof(HWND)*m_monitorCount);
	int width = 0;
	int height = 0;
	int xTemp=0;
	int yTemp=0;
	HDC hDC = NULL;
	tagRECT rect;

	RegisterClassEx(&m_wc);
	char buffer[512];
	int i = 0;
	for(vector<BigScreenMonitor>::iterator iter = m_bigScreenSetParam.monitors.begin();
		iter !=  m_bigScreenSetParam.monitors.end();
		++iter)
	{
		width = iter->width;
		height =iter->height;
		xTemp=iter->BigScreenX;
		yTemp=iter->BigScreenY;
		i = iter->index;
		m_MonitorParam[i].hWnd = createWindow(xTemp,yTemp,width,height,m_wc.hInstance);
		if(NULL == m_MonitorParam[i].hWnd)
		{ 
			printf("create window failed! index = %d\n",i);
			continue;
		}

		//::MoveWindow(m_phWnd[i],m_ScreenRect[i].left,m_ScreenRect[i].top,width,height,true);
		//::SetWindowPos(m_phWnd[i],HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
		//::ShowWindow(m_phWnd[i],SW_SHOW);
	}
	m_bWindowsCreated = true;
}

HWND SOA::Mirror::RPC::WindowHandles::createWindow( int x,int y,int width,int height,HINSTANCE hInstance )
{
	HWND hWnd = CreateWindow("WindowHandles", "WindowHandles", WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_POPUP, //WS_OVERLAPPEDWINDOW//WS_POPUPWINDOW
		x, y, width, height, NULL, NULL, hInstance, NULL);
	return hWnd;
}

void SOA::Mirror::RPC::WindowHandles::getWindowCursorThread()
{
	//WindowHandles *wh=(WindowHandles *)para;
	int bShow=-1;
	while(m_bRuning)
	{
		int sleepCoursorT=getCursorSleepTime(this);
		if((sleepCoursorT>5)&&(bShow!=1))
		{
			for(int i=0;i<getMonitorCount();i++)
			{
				cursorStat=false;
				PostMessage(m_MonitorParam[i].hWnd,88888,0,0);
			}

			bShow=1;
		}
		else if((sleepCoursorT<5)&&(bShow!=0))
		{
			for(int i=0;i<getMonitorCount();i++)
			{
				cursorStat=true;
				PostMessage(m_MonitorParam[i].hWnd,88889,0,0);
			}
			bShow=0;
		}
		Sleep(900);
	}
	m_bWindowCursorThreadExit=true;
}

void SOA::Mirror::RPC::WindowHandles::initAllMonitorInfo()
{
	MonitorDisplayInfoList infoList;
	infoList.count = 100;
	infoList.pBuf = (char*)malloc(100*sizeof(MonitorDisplayInfo));
	if( getAllMonitorDispalyInfo(infoList)==0 )
	{
		for(int i=0; i<infoList.count; i++)
		{
			MonitorDisplayInfo* info = (MonitorDisplayInfo*)infoList.pBuf + i;
			m_MonitorParam[m_monitorCount].rect = info->rect;
			m_MonitorParam[m_monitorCount].hMonitor = info->handle;
			this->m_monitorCount++;
		}
	}
	free(infoList.pBuf);
}



void WindowHandles::showBigScreenSetting()
{
	if(0 >= m_monitorCount)
		return;
	//if(NULL == m_phWnd)
	//	return;
	int width = 0;
	int height = 0;
	HDC hDC = NULL;
	tagRECT rect;
	HFONT font;
	HFONT old_font;

	char buffer[512];
	int top_offset = 0;
	int left_offset = 0;
	int line_height = 0;
	for(int i=0; i<m_monitorCount; i++)
	{
		//width = m_ScreenRect[i].right - m_ScreenRect[i].left;
		//height = m_ScreenRect[i].bottom - m_ScreenRect[i].top;
		width = m_MonitorParam[i].rect.right - m_MonitorParam[i].rect.left;
		height = m_MonitorParam[i].rect.bottom - m_MonitorParam[i].rect.top;
		top_offset = height / 8;
		left_offset = width / 16;
		line_height = (height - top_offset * 2) / 4;
		if(NULL == m_MonitorParam[i].hWnd)
		{ 
			printf("window not be created! index = %d\n",i);
			continue;
		}

		hDC = ::GetDC(m_MonitorParam[i].hWnd);
		if(NULL == hDC)
		{
			printf("get HDC of hWnd failed! index = %d\n",i);
			continue;;
		}

		font = ::CreateFont(
			(width - left_offset * 2) / 25,//line_height,               // nHeight
			0,                         // nWidth
			0,                         // nEscapement
			0,                         // nOrientation
			FW_NORMAL,                 // nWeight
			FALSE,                     // bItalic
			FALSE,                     // bUnderline
			0,                         // cStrikeOut
			ANSI_CHARSET,              // nCharSet
			OUT_DEFAULT_PRECIS,        // nOutPrecision
			CLIP_DEFAULT_PRECIS,       // nClipPrecision
			DEFAULT_QUALITY,           // nQuality
			DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
			("Arial"));                 // lpszFacename

		old_font = (HFONT)::SelectObject(hDC,font);
		::SetBkMode(hDC,TRANSPARENT);
		::SetTextColor(hDC,0x00ffffff);

		rect.left = left_offset;
		rect.top = top_offset;
		rect.right = width;
		rect.bottom = rect.top + line_height;
		sprintf(buffer,"%s: %d X %d","分辨率",width,height);
		::DrawText(hDC,buffer,-1,&rect,DT_NOCLIP);

		rect.left = left_offset;
		rect.top = top_offset + line_height;
		rect.right = width;
		rect.bottom = rect.top + line_height;
		sprintf(buffer,"%s: %d","显示器编号",i);
		::DrawText(hDC,buffer,-1,&rect,DT_NOCLIP);

		rect.left = left_offset;
		rect.top = top_offset + line_height * 2;
		rect.right = width;
		rect.bottom = rect.top + line_height;
		sprintf(buffer,"%s:","配置信息");
		::DrawText(hDC,buffer,-1,&rect,DT_NOCLIP);

		for(vector<BigScreenMonitor>::iterator iter = m_bigScreenSetParam.monitors.begin();
			iter != m_bigScreenSetParam.monitors.end();
			++iter)
		{
			if(iter->index == i)
			{
				rect.left = left_offset + 100;
				rect.top = top_offset + line_height * 3;
				rect.right = width;
				rect.bottom = rect.top + line_height;
				sprintf(buffer,"index=%d, BigScreenX=%d, BigScreenY=%d",i,iter->BigScreenX,iter->BigScreenY);
				::DrawText(hDC,buffer,-1,&rect,DT_NOCLIP);
				break;
			}
		}

		::SelectObject(hDC,old_font);
		::ReleaseDC(m_MonitorParam[i].hWnd,hDC);

		::DeleteObject(font);
	}
}

//static
DWORD WindowHandles::getWindowMessageThread(void *pParam)
{
	WindowHandles *pThis = (WindowHandles*)pParam;
	pThis->getWindowMessageThreadCallback();
	return 0;
}

void WindowHandles::getWindowMessageThreadCallback()
{
	if((1==m_bigScreenSetParam.modeType)||(0==m_bigScreenSetParam.modeType))//服务器版本
	{
		createWindow();
	}
	else if(3==m_bigScreenSetParam.modeType)//WPF版本
	{
		createUserWindow();
	}

	//::ShowCursor(false);
	BOOL fGotMessage;
	MSG msg = {0};
	if(NULL==m_hWindowCursorThread)
	{
		m_bWindowCursorThreadExit=false;
		m_hWindowCursorThread=::CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)controlCursor,this,0,NULL);
	}
	ShowCursor(false);
	while ((fGotMessage = GetMessage(&msg, (HWND) NULL, 0, 0)) != 0 && fGotMessage != -1) 
	{
		if(msg.message==88888)
		{
			ShowCursor(false);
	
		}
		else if (msg.message==88889)
		{
			ShowCursor(true);
		}
		//ShowCursor(true);
		//printf("===%d==message=%x==\n",fGotMessage,msg.message);
		if(false == m_bRuning)
			break;
		if(WM_QUIT == msg.message)
		{
			printf("===============quit============\n");
		}
		if(WM_CLOSE == msg.message)
		{
			printf("===============close============\n");
			break;
		}
		//if(PeekMessage(&msg,NULL,0,0,PM_REMOVE|PM_QS_PAINT))
		{
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		}

	}
	m_bWindowMessageThreadExit = true;
}

void WindowHandles::showCursor(bool bShow)
{
	::ShowCursor(bShow);
}

void WindowHandles::showWindow(bool bShow)
{
	int i = 0;
	if(true == bShow)
	{
		for(vector<BigScreenMonitor>::iterator iter = m_bigScreenSetParam.monitors.begin();
			iter !=  m_bigScreenSetParam.monitors.end();
			++iter)
		{
			i = iter->index;
			if(iter->index >= m_monitorCount)
				continue;
			if(NULL == m_MonitorParam[i].hWnd)
			{ 
				continue;
			}
			::ShowWindow(m_MonitorParam[i].hWnd,SW_SHOW);			
		}
	}
	else
	{
		for(vector<BigScreenMonitor>::iterator iter = m_bigScreenSetParam.monitors.begin();
			iter !=  m_bigScreenSetParam.monitors.end();
			++iter)
		{
			i = iter->index;
			if(iter->index >= m_monitorCount)
				continue;
			if(NULL == m_MonitorParam[i].hWnd)
			{ 
				continue;
			}
			::ShowWindow(m_MonitorParam[i].hWnd,SW_HIDE);			
		}
	}
}

void WindowHandles::brindWindowsToTop(bool bTop)
{
	int i = 0;
	if(true == bTop)
	{
		for(vector<BigScreenMonitor>::iterator iter = m_bigScreenSetParam.monitors.begin();
			iter !=  m_bigScreenSetParam.monitors.end();
			++iter)
		{
			i = iter->index;
			if(iter->index >= m_monitorCount)
				continue;
			if(NULL == m_MonitorParam[i].hWnd)
			{ 
				continue;
			}
			::SetWindowPos(m_MonitorParam[i].hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);//|SWP_NOACTIVATE);		
		}
	}
	else
	{
		for(vector<BigScreenMonitor>::iterator iter = m_bigScreenSetParam.monitors.begin();
			iter !=  m_bigScreenSetParam.monitors.end();
			++iter)
		{
			i = iter->index;
			if(iter->index >= m_monitorCount)
				continue;
			if(NULL == m_MonitorParam[i].hWnd)
			{ 
				continue;
			}
			::SetWindowPos(m_MonitorParam[i].hWnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);		
		}
	}
}


int	WindowHandles::createWindows()
{
	m_bWindowMessageThreadExit = false;
	m_bWindowsCreated = false;
	//createWindow();

	m_hWindowMessageThread = ::CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)getWindowMessageThread,this,0,NULL);
	if(NULL == m_hWindowMessageThread)
	{
		m_bWindowMessageThreadExit = true;
	}

	if(false == m_bWindowMessageThreadExit)
	{
		while(false == m_bWindowsCreated)
		{
			Sleep(10);
		}
	}
	return 0;
}

HMONITOR WindowHandles::getMonitor(int index,tagRECT &rect)
{
	if(index >= m_monitorCount)
		return NULL;
	rect = m_MonitorParam[index].rect;
	return m_MonitorParam[index].hMonitor;
}

int WindowHandles::setScreenFrequency(int monitorCount,int frequency)
{
	if(-1==frequency)
	{
		return -1;
	}

	bool result=false;

	for(int i=0;i<monitorCount;i++)
	{
		DISPLAY_DEVICE device;
		device.cb= sizeof(device);
		result=EnumDisplayDevices(NULL,i,&device,0);

		DEVMODEA currentDevmode;
		memset(&currentDevmode,0,sizeof(DEVMODEA));
		currentDevmode.dmSize = sizeof(DEVMODEA);
		EnumDisplaySettingsEx((LPCSTR)device.DeviceName,ENUM_CURRENT_SETTINGS,(LPDEVMODEA)&currentDevmode,NULL);
		

		//暂时，2012-11-28 ，将机器的刷新率控制在60以下
		if((currentDevmode.dmDisplayFrequency > 60)||(frequency>60))
		{
			frequency = 60;
		}
		if(currentDevmode.dmDisplayFrequency == frequency)
		{
			printf("Current displayFrequency is %d Hz!\n",frequency);
			continue;
		}

		//修改刷新率
		//LPDEVMODEW devmode;
		DEVMODEA devmode;
		memset(&devmode,0,sizeof(devmode));
		devmode.dmSize = sizeof(devmode);
		devmode.dmPelsWidth=m_MonitorParam[i].rect.right-m_MonitorParam[i].rect.left;
		devmode.dmPelsHeight=m_MonitorParam[i].rect.bottom-m_MonitorParam[i].rect.top;
		devmode.dmDisplayFrequency=frequency;
		devmode.dmBitsPerPel=32;
		devmode.dmFields=DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFREQUENCY|DM_BITSPERPEL;
		long ret=ChangeDisplaySettingsEx((LPCSTR)device.DeviceName,(LPDEVMODEA)&devmode,NULL,CDS_UPDATEREGISTRY|CDS_GLOBAL,NULL);
		switch(ret)
		{
			case DISP_CHANGE_BADDUALVIEW:
				printf("ChangeDisplaySettingsExW Error: DISP_CHANGE_BADDUALVIEW\n");
				break;
			case DISP_CHANGE_SUCCESSFUL:
				printf("ChangeDisplaySettingsExW: DISP_CHANGE_SUCCESSFUL\n");
				break;
			case  DISP_CHANGE_RESTART:
				printf("ChangeDisplaySettingsExW Error: DISP_CHANGE_RESTART\n");
				break;
			case  DISP_CHANGE_BADFLAGS:
				printf("ChangeDisplaySettingsExW Error: DISP_CHANGE_BADFLAGS\n");
				break;
			case  DISP_CHANGE_NOTUPDATED:
				printf("ChangeDisplaySettingsExW Error: DISP_CHANGE_NOTUPDATED\n");
				break;
			case DISP_CHANGE_BADPARAM:
				printf("ChangeDisplaySettingsExW Error: DISP_CHANGE_BADPARAM\n");
				break;
			case  DISP_CHANGE_FAILED:
				printf("ChangeDisplaySettingsExW Error: DISP_CHANGE_FAILED\n");
				break;
			case DISP_CHANGE_BADMODE:
				printf("ChangeDisplaySettingsExW Error: DISP_CHANGE_BADMODE\n");
				break;
		}
		if(0!=ret)
		{
			printf("ChangeDisplaySettings Failed!\n");
		}
	}
	return 0;

}