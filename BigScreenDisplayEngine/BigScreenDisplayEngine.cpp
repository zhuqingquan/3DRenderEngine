// BigScreenDisplayEngine.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "DxRender.h"
#include "DisplayElement.h"
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <assert.h>
#include "WindowModel.h"
#include "RenderDrawing.h"
#include "BigViewportPartition.h"
#include "BigView.h"
#include "d3dAdapterOutputEnumerator.h"
#include "IndependentBigScreenBackground.h"
#include "MergedBigScreenBackground.h"
#include "Screen.h"
#include "ScreenRender.h"
#include "BigViewport.h"
#include "CommandShell.h"
#include "RawFileSource.h"

using namespace SOA::Mirror::Render;
using namespace std;

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	switch( msg )
	{
	// WM_ACTIVATE is sent when the window is activated or deactivated.  
	// We pause the game when the window is deactivated and unpause it 
	// when it becomes active.  
	case WM_PAINT:
		BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		return 0;
	case WM_ACTIVATE:
		return 0;

	// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		return 0;

	// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		return 0;

	// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
	// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		return 0;
 
	// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	// The WM_MENUCHAR message is sent when a menu is active and the user presses 
	// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
        // Don't beep when we alt-enter.
        return MAKELRESULT(0, MNC_CLOSE);

	// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200; 
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		return 0;
	case WM_MOUSEMOVE:
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

HWND g_hwnd = NULL;

DWORD WINAPI peekMsgThreadWork(LPVOID param)
{
	HINSTANCE instance = GetModuleHandle(NULL);
	g_hwnd = CreateWindow(_T("D3DWndClassName"), _T("DisplayWindow"), 
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP/*WS_OVERLAPPEDWINDOW*/, 
		0, 0/*CW_USEDEFAULT, CW_USEDEFAULT*/, 
		1366, 768, 0, 0, instance, 0); 
	if( !g_hwnd )
	{
		MessageBox(0, _T("CreateWindow Failed."), 0, 0);
		return false;
	}

	//ShowWindow(mhMainWnd, SW_SHOW);
	//UpdateWindow(mhMainWnd);

	MSG msg = {0};
 
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	LARGE_INTEGER startTime;
	QueryPerformanceCounter(&startTime);
	long drawFrameCount = 0;
	LONGLONG lastTotalTimeCount = 0;

	LONGLONG lastTimeDraw = 0;
	while(msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if(PeekMessage( &msg, g_hwnd, 0, 0, PM_REMOVE ))
		//if(GetMessage(&msg, mhMainWnd, 0, 0)!=0)
		{
            TranslateMessage( &msg );
            DispatchMessage( &msg );
		}
    }

	return (int)msg.wParam;
}

HANDLE startThreadToPeekMessage()
{
	HANDLE thread = CreateThread(NULL, 0, peekMsgThreadWork, 0, 0, 0);
	assert(thread);
	return thread;
}

DWORD WINAPI renderTimerThreadWork(LPVOID param)
{
	//HANDLE timeHandle = (HANDLE) param;
	HANDLE* pTimeHandle = (HANDLE*)param;
	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	while(true)
	{
		Sleep(16);
		//ReleaseSemaphore(timeHandle, 4, 0);
		for(int i=0; i<4; i++)
		{
			SetEvent(*(pTimeHandle+i));
		}
	}

	D3D_FEATURE_LEVEL curFeatureLv = D3D_FEATURE_LEVEL_9_1;
	D3D_FEATURE_LEVEL featureLvs[] = {
		//D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};
	ID3D11Device* device = NULL;
	ID3D11DeviceContext* contex = NULL;
	IDXGISwapChain* swapchain = NULL;

	HRESULT hr = D3D11CreateDevice(
									NULL,                 // use selected adapter
									D3D_DRIVER_TYPE_HARDWARE,	//adapter is No NULL
									0,							// no software device
									createDeviceFlags, 
									featureLvs, ARRAYSIZE(featureLvs),// default feature level array
									D3D11_SDK_VERSION,
									&device,
									&curFeatureLv,
									&contex);
	if(S_OK!=hr)
	{
#ifdef _DEBUG
		printf("Error in D3D11 init : faile to Create device with param Adapter(NULL) CreateFlag(%d) ErrorCode=%d\n",
			createDeviceFlags, (int)hr);
#endif
		return -2;
	}
	WindowModel* wm = new WindowModel();
	RECT winRect = {0, 0, 32, 32};
	wm->setWindowRect(winRect);
	HWND hWnd = wm->createWindows();
	//wm->showWindow();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width  = 32;
	sd.BufferDesc.Height = 32;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;//DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount  = 1;
	sd.OutputWindow = hWnd;
	sd.Windowed     = true;
	sd.SwapEffect   = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags        = 0;
	sd.SampleDesc.Count   = 1;
	sd.SampleDesc.Quality = 0;

	// To correctly create the swap chain, we must use the IDXGIFactory that was
	// used to create the device.  If we tried to use a different IDXGIFactory instance
	// (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
	// This function is being called with a device from a different IDXGIFactory."
	IDXGIDevice* dxgiDevice = 0;
	if(S_OK!=device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice))
		return -5;	      
	IDXGIAdapter* dxgiAdapter = 0;
	if(S_OK!=dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter))
		return -6;
	IDXGIFactory* dxgiFactory = 0;
	if(S_OK!=dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory))
		return -7;

	if(S_OK!=dxgiFactory->CreateSwapChain(device, &sd, &swapchain))
	{
#ifdef _DEBUG
		printf("Error in DxRender_D3D11::init : Create Swap Chain faile.(Width=%d, Height=%d, SampleCount=%d, SampleQuality=%d)\n",
			sd.BufferDesc.Width, sd.BufferDesc.Height, sd.SampleDesc.Count, sd.SampleDesc.Quality);
#endif
		return -8;
	}
	
	ReleaseCOM(dxgiDevice);
	ReleaseCOM(dxgiAdapter);
	ReleaseCOM(dxgiFactory);
	LARGE_INTEGER freq;
	LARGE_INTEGER cur;
	LONGLONG lastTime = 0;
	LONGLONG nowTime = 0;
	QueryPerformanceFrequency(&freq);

	while(true)
	{
		QueryPerformanceCounter(&cur);
		nowTime = cur.QuadPart * 1000 / freq.QuadPart;
		if(lastTime!=0)
		{
			int interval = nowTime - lastTime;
			//printf("interval %d\n", interval);
		}
		lastTime = nowTime;
		swapchain->Present(1, 0);
		//ReleaseSemaphore(timeHandle, 4, 0);
		for(int i=0; i<4; i++)
		{
			SetEvent(*(pTimeHandle+i));
		}
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	HINSTANCE instance = GetModuleHandle(NULL);
	WNDCLASS wc;
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = MainWndProc; 
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = instance;
	wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = _T("D3DWndClassName");

	if( !RegisterClass(&wc) )
	{
		//MessageBox(0, L"RegisterClass Failed.", 0, 0);
		//return false;
	}

	std::vector<IDXGIAdapter*> adpts;
	if (S_OK != DXGI_getAdapters(adpts))
	{
		printf("Error in get adapters information.\n");
		return -1;
	}
	//打印所有的显示器信息
	std::vector<IDXGIOutput*> allOutputs;//保存所有的IDXGIOutput信息
	size_t outputCount = 0;
	for (size_t i = 0; i < adpts.size(); i++)
	{
		IDXGIAdapter* pAdpt = adpts[i];
		assert(pAdpt);
		std::vector<IDXGIOutput*> outputsInCurAdpt;
		if (S_OK != DXGI_getOutputs(pAdpt, outputsInCurAdpt))
		{
			printf("Error in get outputs from adapter %d\n", i);
			continue;
		}
		if (outputsInCurAdpt.size() <= 0)
		{
			printf("Their has not Output in Adapter %d\n", i);
			continue;
		}
		printf("显卡 %d 中的所有显示器:\n", i);
		for (size_t indexOfOutput = 0; indexOfOutput < outputsInCurAdpt.size(); indexOfOutput++)
		{
			IDXGIOutput* curOutput = outputsInCurAdpt[indexOfOutput];
			assert(curOutput);
			DXGI_OUTPUT_DESC outputDesc;
			if (S_OK != curOutput->GetDesc(&outputDesc))
			{
				printf("Error in get output description of output %d in adapter %d\n", indexOfOutput, i);
				continue;
			}
			printf("\t显示器 %d Left=%d Top=%d Right=%d Bottom=%d\n", outputCount,
				outputDesc.DesktopCoordinates.left, outputDesc.DesktopCoordinates.top, outputDesc.DesktopCoordinates.right, outputDesc.DesktopCoordinates.bottom);
			allOutputs.push_back(curOutput);
			outputCount++;
		}
	}

	if (argc < 2)
	{
		printf("Command Argument invalid.\n");
		return -3;
	}
	ifstream cfgFileStream(argv[1], ios_base::in);
	doCommand(cfgFileStream);

	//等待用户输入使用哪些显示器显示视频
	printf("请输入显示器序号选择需要创建窗口的显示器（格式如 0(0_0)，1(1_0)，2(0_1)）：");
	std::string inputString;
	cin >> inputString;

	SOA::Mirror::Render::ScreenConfig scfg;
	int posStart = 0;
	int posEnd = posStart;
	while (inputString.npos != (posEnd = inputString.find(',', posStart)))
	{
		if (posStart < posEnd)
		{
			std::string num = inputString.substr(posStart, posEnd);
			posStart = posEnd + 1;
			int outputIndex = -1;
			int posX = -1;
			int posY = -1;
			sscanf(num.c_str(), "%d(%d_%d)", &outputIndex, &posX, &posY);
			if (outputIndex == -1 || -1==posX || -1==posY || outputIndex>=allOutputs.size())
				continue;
			ScreenCellConfig cellcfg;
			cellcfg.output = allOutputs[outputIndex];
			cellcfg.posX = static_cast<float>(posX);
			cellcfg.posY = static_cast<float>(posY);
			scfg.width = scfg.width < posX + 1 ? posX + 1 : scfg.width;
			scfg.height = scfg.height < posY + 1 ? posY + 1 : scfg.height;
			scfg.screenCellCfg.push_back(cellcfg);
		}
	}

	//SOA::Mirror::Render::BigScreenBackground* bg = new IndependentBigScreenBackground(
	//"C:\\Users\\Public\\Pictures\\Sample Pictures\\Penguins.jpg",
	//3, 3, SOA::Mirror::Render::Size(1366/2, 768/2), zRender::PIXFMT_YUV420P);
	BigScreenBackground* bg = new MergedBigScreenBackground(
		"C:\\Users\\Public\\Pictures\\Sample Pictures\\Penguins.jpg",
		3, 3, SOA::Mirror::Render::Size(1366 / 2, 768 / 2), zRender::PIXFMT_YUV420P);
	//bg->allocBackgroundContentForBigscreenCell();

	Screen* screen = NULL;
	try
	{
		screen = new Screen(scfg, NULL/*bg*/);
	}
	catch (const std::exception& ex)
	{
		printf("Create Screen failed.(%s)\n", ex.what());
		return -2;
	}
	catch (...)
	{
		printf("Create Screen failed.(Unknow Exception)\n");
		return -3;
	}

	//HANDLE peedMsgTh = startThreadToPeekMessage();
	HANDLE timerHandle = CreateSemaphore(NULL, 0, 4, NULL);
	std::vector<HANDLE> timerHandleVec;
	/*timerHandleVec.push_back(CreateEvent(NULL, false, false, NULL));
	timerHandleVec.push_back(CreateEvent(NULL, false, false, NULL));
	timerHandleVec.push_back(CreateEvent(NULL, false, false, NULL));
	timerHandleVec.push_back(CreateEvent(NULL, false, false, NULL));*/

	std::vector<ScreenRender*> allScreenRender = screen->getScreenRender();
	for (size_t i = 0; i < allScreenRender.size(); i++)
	{
		timerHandleVec.push_back(allScreenRender[i]->getTimerEventHandle());
	}
	
	/*WindowModel* wm = new WindowModel();//左上
	RECT winRect = {0, 0, 683, 384};
	wm->setWindowRect(winRect);
	HWND hWnd = wm->createWindows();
	wm->showWindow();

	WindowModel* wm2 = new WindowModel();//右上
	RECT winRect2 = {683, 0, 1366, 384};
	wm2->setWindowRect(winRect2);
	HWND hWnd2 = wm2->createWindows();
	wm2->showWindow();

	WindowModel* wm3 = new WindowModel();//左下
	RECT winRect3 = {0, 384, 683, 768};
	wm3->setWindowRect(winRect3);
	HWND hWnd3 = wm3->createWindows();
	wm3->showWindow();

	WindowModel* wm4 = new WindowModel();//右下
	RECT winRect4 = {683, 384, 1366, 768};
	wm4->setWindowRect(winRect4);
	HWND hWnd4 = wm4->createWindows();
	wm4->showWindow();*/
	
	//HANDLE timerThread = CreateThread(NULL, 0, renderTimerThreadWork, timerHandle, 0, 0);
	HANDLE timerThread = CreateThread(NULL, 0, renderTimerThreadWork, &timerHandleVec[0], 0, 0);
	Sleep(1000);
	//system("pause");

	/*Sleep(1000);

	SOA::Mirror::Render::RenderDrawing* rd3 = new SOA::Mirror::Render::RenderDrawing(hWnd3, 0.0, 2, 1, 3, bg);
	//rd3->start(timerHandle);
	rd3->start(timerHandleVec[2]);
	//system("pause");

	SOA::Mirror::Render::RenderDrawing* rd = new SOA::Mirror::Render::RenderDrawing(hWnd, 0.0, 1.0, 1, 2, bg);
	//rd->start(timerHandle);
	rd->start(timerHandleVec[0]);
	//system("pause");

	SOA::Mirror::Render::RenderDrawing* rd2 = new SOA::Mirror::Render::RenderDrawing(hWnd2, 1.0, 1, 2, 2, bg);
	//rd2->start(timerHandle);
	rd2->start(timerHandleVec[1]);
	//system("pause");

	SOA::Mirror::Render::RenderDrawing* rd4 = new SOA::Mirror::Render::RenderDrawing(hWnd4, 1.0, 2, 2, 3, bg);
	//rd4->start(timerHandle);
	rd4->start(timerHandleVec[3]);*/
//	system("pause");
	ScreenRender* sr = screen->getScreenRender(0, 0);
	RenderDrawing* rd = sr->getRenderDrawing();
	zRender::DxRender* dxrender = rd->getDxRender();

	BigViewport* backVideoViewport = screen->createViewport(zRender::RECT_f(0.3, 1, 0.3, 1), 150);
	if (backVideoViewport)
	{
		const TCHAR* fileName = _T("D:\\InsideMoveVtc.yuv");
		zRender::RawFileSource* videoFileSrc = new zRender::RawFileSource(dxrender);
		if (videoFileSrc->open(fileName, zRender::PIXFMT_YUY2, 1920, 1080))
		{
			BigView* videoView = videoFileSrc->createSourceView();
			if (videoView)
			{
				backVideoViewport->attachView(videoView);
			}
			videoFileSrc->start(25);
		}
		else
		{
			wprintf(_T("Create RawFileSource failed.[%s]\n"), fileName);
		}
	}

	zRender::RawFileSource* fileSrc = new zRender::RawFileSource(dxrender);
	//fileSrc->open(_T("D:\\InsideMoveVtc.yuv"), zRender::PIXFMT_YUY2, 1920, 1080);
	fileSrc->open(_T("D:\\测试用视频\\transImag_500_282.rgb32"), zRender::PIXFMT_R8G8B8A8, 500, 282);
	
	BigView* view = fileSrc->createSourceView();
	//BigView* view = new BigView(zRender::RECT_f(0.0, 1, 0.0, 1));

	//zRender::RECT_f regOfBigScreen(0.0, 1.0, 1, 2);
	//zRender::RECT_f regOfBigViewport(0, 0.5, 0, 0.5);
	//BigViewportPartition* bvv = new	BigViewportPartition(regOfBigScreen, regOfBigViewport, rd);
	//bvv->setZIndex(1);
	//rd->addBigViewportPartition(bvv);
	//bvv->attachBigView(view);

	//zRender::RECT_f regOfBigScreen1(1.0, 2.0, 1, 2);
	//zRender::RECT_f regOfBigViewport1(0.5, 1, 0, 0.5);
	//BigViewportPartition* bvv2 = new	BigViewportPartition(regOfBigScreen1, regOfBigViewport1, rd2);
	//bvv2->setZIndex(1);
	//rd2->addBigViewportPartition(bvv2);
	//bvv2->attachBigView(view);

	//zRender::RECT_f regOfBigScreen2(0.0, 1.0, 2, 3);
	//zRender::RECT_f regOfBigViewport2(0, 0.5, 0.5, 1);
	//BigViewportPartition* bvv3 = new	BigViewportPartition(regOfBigScreen2, regOfBigViewport2, rd3);
	//bvv3->setZIndex(1);
	//rd3->addBigViewportPartition(bvv3);
	//bvv3->attachBigView(view);

	BigViewport* bvp = screen->createViewport(zRender::RECT_f(0, 0.7, 0, 0.7), 99);
	if (bvp)
		bvp->attachView(view);

	/*zRender::RECT_f regOfBigScreen3(1.0, 2.0, 2, 3);
	zRender::RECT_f regOfBigViewport3(0.5, 1, 0.5, 1);
	BigViewportPartition* bvv4 = new	BigViewportPartition(regOfBigScreen3, regOfBigViewport3, rd4);
	bvv4->setZIndex(99);
	rd4->addBigViewportPartition(bvv4);
	bvv4->attachBigView(view);

	zRender::RECT_f regOfBigScreen4(1.25, 1.75, 2.25, 2.75);
	zRender::RECT_f regOfBigViewport4(0.0, 1, 0.0, 1);
	BigViewportPartition* bvv5 = new	BigViewportPartition(regOfBigScreen4, regOfBigViewport4, rd4);
	bvv5->setZIndex(99);
	rd4->addBigViewportPartition(bvv5);
	bvv5->attachBigView(view);

	int zIndex = 100;
	printf("Set zIndex to 100, ");
	while (true)
	{
		system("pause");
		bvv5->setZIndex(zIndex--);
	}*/
	
	//zRender::DxRender* render = new zRender::DxRender();
	//render->init(hWnd);
	//render->setVisibleRegion(zRender::RECT_f(1.0, 2.0, 1.0, 2.0));

	//zRender::DisplayElement* displayEle = render->createDisplayElement(zRender::RECT_f(1.0, 2.0, 1.0, 2.0));

	//zRender::Vertex ver[4];
	//ver[0].Pos = XMFLOAT3(0.0, 0.0, 0.0);
	//ver[0].Tex = XMFLOAT2(0.0, 0.0);
	//ver[1].Pos = XMFLOAT3(1.0, 0.0, 0.0);
	//ver[1].Tex = XMFLOAT2(1.0, 0.0);
	//ver[2].Pos = XMFLOAT3(0.0, 1.0, 0.0);
	//ver[2].Tex = XMFLOAT2(0.0, 1.0);
	//ver[3].Pos = XMFLOAT3(1.0, 1.0, 0.0);
	//ver[3].Tex = XMFLOAT2(1.0, 1.0);
	//UINT index[6] = {0, 1, 2, 2, 1, 3};
	//zRender::VertexVector verVec(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//verVec.addVertexs(ver, 4, index, 6);
	//displayEle->setVertex(&verVec);

	//RECT winrect = wm->getWindowRect();
	//int picWidth = 720;//winrect.right - winrect.left;
	//int picHeight = 576;//winrect.bottom - winrect.top;
	//displayEle->setTexture(zRender::YUVFORMAT_PACKED_YUYV, picWidth, picHeight, NULL);

	//render->draw(displayEle);

	//render->present(0);
	fileSrc->start(25);

	printf("Press Any key to disattachView  ");
	system("pause");

	//bvv->disattachView();
	//bvv2->disattachView();
	//bvv3->disattachView();
	/*bvv4->disattachView();
	bvv5->disattachView();
	printf("Press Any key to delete view  ");
	system("pause");

	delete view;
	view = NULL;
	printf("Press Any key to Notify bigviewportpartition to release  ");
	system("pause");

	//bvv->notifyToRelease();
	//bvv2->notifyToRelease();
	//bvv3->notifyToRelease();
	bvv4->notifyToRelease();
	bvv5->notifyToRelease();
	printf("Press Any key to Stop RenderDrawing  ");
	system("pause");

	rd->stop();
	rd2->stop();
	rd3->stop();
	rd4->stop();
	printf("Press Any key to delete RenderDrawing  ");
	system("pause");

	delete rd;
	delete rd2;
	delete rd3;
	delete rd4;

	printf("Press Any key to delete other objects  ");
	system("pause");
	delete bg;
	delete wm;
	delete wm2;
	delete wm3;
	delete wm4;*/
	return 0;
}
