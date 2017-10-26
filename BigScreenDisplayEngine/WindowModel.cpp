#include "WindowModel.h"
#include <stdio.h>
#include <tchar.h>
#define WM_TRAY_NOTIFY (WM_USER + 8800)
#include <assert.h>

using namespace SOA::Mirror::RPC;

HICON g_hIcons = NULL;
LRESULT WINAPI WindowHandlesMsgProcT(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	switch(msg)
	{
	case WM_CLOSE: 
		DestroyWindow(hWnd);
		return 0;
	case WM_DESTROY :
		{
			::PostMessage(hWnd,WM_QUIT,NULL,NULL);
			return 0;
		}
	case WM_PAINT:
		//ValidateRect(hWnd, NULL);
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		return 0;
	case WM_SIZE:
	case WM_GETMINMAXINFO:
		return 0;
	case WM_TRAY_NOTIFY:
		switch( lParam )
		{
		case WM_LBUTTONDBLCLK:
			{
				TCHAR * strTile=new TCHAR[255];
				GetConsoleTitle(strTile,255);
				HWND ret=FindWindow(NULL,strTile);
				ShowWindow(ret,SW_SHOWNOACTIVATE);
				::SetWindowPos(ret,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

			}
			//do something
			break;
		}
		return 0;
	case WM_GETICON:
		return 0;
		if(ICON_BIG == wParam)
		{
			printf("-----get big icon-----\n");
			return (LRESULT)g_hIcons;
		}
		else if(ICON_SMALL == wParam)
		{
			printf("-----get small icon-----\n");
			return (LRESULT)g_hIcons;
		}

	}
//  	if(WM_CLOSE == msg/* || WM_QUIT == msg*/)
//  	{
//  		//::PostMessage(hWnd,WM_QUIT,NULL,NULL);
//  		DestroyWindow(hWnd);
//  		return 0;
//  	}
// 	if(WM_PAINT == msg)
// 	{
// 		ValidateRect(hWnd, NULL);
// 		return 0;
// 	}
// 	if(WM_SIZE == msg || WM_GETMINMAXINFO == msg)
// 		return 0;
// 	//if(WM_DESTROY == msg)
// 	//	return 0;
// 	if(WM_TRAY_NOTIFY==msg)
// 	{
// 		switch( lParam )
// 		{
// 		case WM_LBUTTONDBLCLK:
// 			{
// 				char * strTile=new char[255];
// 				GetConsoleTitle(strTile,255);
// 				HWND ret=FindWindowA(NULL,strTile);
// 				ShowWindow(ret,SW_SHOWNOACTIVATE);
// 				::SetWindowPos(ret,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
// 
// 			}
// 			//do something
// 			break;
// 		}
// 		return 0;
// 	}
// 
// 	if(WM_GETICON == msg)
// 	{
// 		return 0;
// 		if(ICON_BIG == wParam)
// 		{
// 			printf("-----get big icon-----\n");
// 			return (LRESULT)g_hIcons;
// 		}
// 		else if(ICON_SMALL == wParam)
// 		{
// 			printf("-----get small icon-----\n");
// 			return (LRESULT)g_hIcons;
// 		}
// 	}
// 	//printf("-----%x-----\n",msg);
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

WindowModel::WindowModel()
{
	m_bRuning = true;

	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WindowHandlesMsgProcT, 0L, 0L,//PRF_NONCLIENT//CS_CLASSDC
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("WindowModels"), NULL };
	g_hIcons = wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	memcpy(&m_wc,&wc,sizeof(WNDCLASSEX));

	m_wc.hbrBackground = (HBRUSH)::GetStockObject(BLACK_BRUSH);//COLOR_BACKGROUND

	m_winRect.left = 20;
	m_winRect.top = 30;
	m_winRect.right = 70;
	m_winRect.bottom = 90;
}

WindowModel::~WindowModel()
{
	while(!m_bWindowMessageThreadExit)
	{
		Sleep(1);
	}
	UnregisterClass(_T("WindowModels"),m_wc.hInstance);
}

void WindowModel::getWindowMessageThreadCallback()
{
	//createWindow()
	createWindow(m_winRect.left,m_winRect.top,m_winRect.right-m_winRect.left,m_winRect.bottom-m_winRect.top);

	//::ShowCursor(false);
	BOOL fGotMessage;
	MSG msg = {0};
	while ((fGotMessage = GetMessage(&msg, m_hwnd, 0, 0)) != 0 && fGotMessage != -1) 
	{ 
		//printf("msg=%d\n",msg);
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

HWND WindowModel::createWindows()
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
	return m_hwnd;
}

DWORD WindowModel::getWindowMessageThread( void *pParam )
{
	WindowModel *pThis = (WindowModel*)pParam;
	pThis->getWindowMessageThreadCallback();
	return 0;
}

HWND WindowModel::createWindow(int w_left,int w_top,int w_width,int w_height)
{
	RegisterClassEx(&m_wc);
	m_hwnd= createWindow_internal(w_left,w_top,w_width,w_height,m_wc.hInstance);
	if(NULL == m_hwnd)
	{ 
		printf("create window failed!\n");
	}
	m_bWindowsCreated = true;
	return m_hwnd;
}

HWND WindowModel::createWindow_internal( int x,int y,int width,int height,HINSTANCE hInstance )
{
	HWND hWnd = CreateWindow(_T("WindowModels"), _T("WindowModels"), WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_POPUP, //WS_OVERLAPPEDWINDOW//WS_POPUPWINDOW
		x, y, width, height, NULL, NULL, hInstance, NULL);
	return hWnd;
}

void WindowModel::showWindow()
{
	ShowWindow(m_hwnd,SW_NORMAL);
}

void WindowModel::hideWindow()
{
	ShowWindow(m_hwnd,SW_HIDE);
}

bool WindowModel::setBackColor( DWORD color )
{
	if(m_hwnd==NULL)
		return false;
	int r = ((color & 0x00ff0000) >> 16);
	int g = ((color & 0x0000ff00) >> 8);
	int b = ( color & 0x000000ff);
	HBRUSH brush = CreateSolidBrush(RGB(r,g,b));
	HDC hDC = ::GetDC(m_hwnd);
	RECT rcBKColor;
	rcBKColor.left =  0;
	rcBKColor.right =  m_winRect.right-m_winRect.left;
	rcBKColor.top =  0;
	rcBKColor.bottom =  m_winRect.bottom-m_winRect.top;
	FillRect(hDC, &rcBKColor, brush);	
	::ReleaseDC(m_hwnd, hDC);
	DeleteObject(brush);
}

bool WindowModel::drawText( const TCHAR* text, const RectCoordinate& rectOfWindow, COLORREF fontColor )
{
	if(text==NULL || lstrlen(text)<=0 || RectCoordinate::justRectParamByNormalized(rectOfWindow)!=0)
	{
		printf("Error in WindowModel::drawText : param invalid.\n");
		return false;
	}
	if(m_hwnd==NULL)	return false;
	int fontHeight = (int)((float)(m_winRect.bottom - m_winRect.top) * (rectOfWindow.bottom - rectOfWindow.top));
	assert(fontHeight);
	HDC hDC = ::GetDC(m_hwnd); 
	if(NULL == hDC)
	{
		printf("Error in WindowModel::drawText : GetDC failed.\n");
		return false;
	}
	HFONT indexfont = ::CreateFont(
		fontHeight,						// nHeight
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
		_T("Arial"));                 // lpszFacename
	HFONT old_font = (HFONT)::SelectObject(hDC,indexfont);
	::SetBkMode(hDC,TRANSPARENT);
	::SetTextColor(hDC,fontColor);

	SIZE sizeOfString;
	GetTextExtentPoint(hDC, text, lstrlen(text), &sizeOfString);
	int toTop = (m_winRect.bottom - m_winRect.top - sizeOfString.cy) / 2;
	toTop = toTop<0 ? 0 : toTop;
	int toLeft = (m_winRect.right - m_winRect.left - sizeOfString.cx) / 2;
	toLeft = toLeft<0 ? 0 : toLeft;
	RECT textRegion = m_winRect;
	textRegion.left += toLeft;
	textRegion.top += toTop;
	::DrawText(hDC, text, -1, &textRegion, DT_NOCLIP);

	::SelectObject(hDC,old_font);
	::ReleaseDC(m_hwnd,hDC);
	::DeleteObject(indexfont);
	return true;
}
