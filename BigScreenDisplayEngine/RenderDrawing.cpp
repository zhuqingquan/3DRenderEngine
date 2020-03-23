#include "RenderDrawing.h"
#include "DxRender.h"
#include "DisplayElement.h"
#include <assert.h>
#include "BigViewportPartition.h"
#include <tchar.h>
#include "inc/TextureResource.h"
#include "ElemDsplModel.h"

using namespace SOA::Mirror::Render;
using namespace zRender;

DWORD WINAPI renderThreadWork(LPVOID param);

RenderDrawing::RenderDrawing(HWND attatchWnd, float ltPointX, float ltPointY, float rbPointX, float rbPointY, BigScreenBackground* background)
	: m_hwnd(attatchWnd), m_render(NULL)
	, m_isRunning(false)
	, m_thread(NULL)
	, m_ltPointX(ltPointX), m_ltPointY(ltPointY), m_rbPointX(rbPointX), m_rbPointY(rbPointY)
	, m_background(background)
	, m_dsplModel(NULL)
{
}

RenderDrawing::~RenderDrawing()
{
	stop();
}

int RenderDrawing::start(HANDLE timerHandle)
{
	if(m_isRunning) 
		return 0;
	if(WaitForSingleObject(timerHandle, 1)==WAIT_FAILED)
		return -2;
	m_timerHandle = timerHandle;
	m_isRunning = true;
	m_thread = CreateThread(NULL, 0, renderThreadWork, this, 0, 0);
	if(NULL==m_thread)
		return -1;
	while(!m_render)
		Sleep(1);
	return 0;
}

void RenderDrawing::stop()
{
	m_isRunning = false;
	WaitForSingleObject(m_thread, INFINITE);
	CloseHandle(m_thread);
	m_thread = NULL;
	return;
}

int RenderDrawing::getWidthInPixel() const
{
	if(NULL==m_hwnd)
		return 0;
	RECT winRect;
	if(!GetWindowRect(m_hwnd, &winRect))
		return -1;
	return winRect.right - winRect.left;
}

int RenderDrawing::getHeightInPixel() const
{
	if(NULL==m_hwnd)
		return 0;
	RECT winRect;
	if(!GetWindowRect(m_hwnd, &winRect))
		return -1;
	return winRect.bottom - winRect.top;
}

int RenderDrawing::addBigViewportPartition(BigViewportPartition* viewportPartition)
{
	if(viewportPartition==NULL)
	{
#ifdef _DEBUG
		printf("Error in RenderDrawing::addBigViewportPartition : invalid param.\n");
#endif
		return -1;
	}
	if(m_render==NULL)
	{
#ifdef _DEBUG
		printf("Error in RenderDrawing::addBigViewportPartition : RenderDrawing Not start yet.\n");
		assert(false);
#endif
		return -2;
	}
	zRender::DisplayElement* de = createDisplayElement(viewportPartition);
	if(NULL==de)
		return -3;
	if(0!=viewportPartition->attachDisplayElement(de))
	{
		m_render->releaseDisplayElement(&de);
		return -4;
	}
	addViewportPartition(viewportPartition);
	return 0;
}

zRender::DisplayElement* RenderDrawing::createDisplayElement(BigViewportPartition* vpPartition)
{
	assert(vpPartition);
	RECT_f regOfBigScreen = vpPartition->getRegOfBigScreen();
	int zIndex = vpPartition->getZIndex();
	DisplayElement* de = m_render->createDisplayElement(regOfBigScreen, zIndex);
	if (m_dsplModel)
	{
		de->setDsplModel(m_dsplModel);
	}
	return de;
}

void RenderDrawing::addViewportPartition(BigViewportPartition* vpPartition)
{
	m_vpPartitions.push_back(vpPartition);
}

#include <fstream>

DWORD WINAPI renderThreadWork(LPVOID param)
{
	RenderDrawing* rd = static_cast<RenderDrawing*>(param);
	if(rd==NULL)
		return -1;
	return rd->doRenderWork();
}

int RenderDrawing::doRenderWork()
{
	zRender::DxRender* render = new zRender::DxRender();
	int ret = -1;
	int width = 1920, height = 1080;
	if( 0!=(ret=render->init(m_hwnd, _T("D:\\代码黑洞\\3DRenderEngine\\DxRender\\FX\\DefaultVideo.fxo"))) )
		//if( 0!=(ret=render->init(width, height, _T("G:\\代码黑洞\\MediaCloudDirector\\mshow_v3.0.1.1_GT_anchor_pb\\bin\\Release\\DefaultVideo.fxo"))) )
	{
#ifdef _DEBUG
		printf("Error in RenderDrawing::doRenderWork : failed to init DxRender.(HWND=%d)\n", (int)m_hwnd);
		assert(false);
#endif
		return -1;
	}
	width = render->getWidth();
	height = render->getHeight();

	if(0!=render->setVisibleRegion(zRender::RECT_f(m_ltPointX, m_rbPointX, m_ltPointY, m_rbPointY)))//1.0, 2.0, 1.0, 2.0
	{
#ifdef _DEBUG
		printf("Error in RenderDrawing::doRenderWork : setVisibleRegion of DxRender failed.(L=%f T=%f R=%f B=%f)\n",
			m_ltPointX, m_ltPointY, m_rbPointX, m_rbPointY);
		assert(false);
#endif
		return -2;
	}

//	render->createOffscreenRenderTarget(1920, 1080);

	if(m_background)
	{
		IDisplayContentProvider* dcp = m_background->getDataContentOfBigscreenCell(m_ltPointX, m_ltPointY);
		RECT_f regForDisplayBackgroun = m_background->getDisplayRegOfBigscreenCell(m_ltPointX, m_ltPointY);
		if(dcp==NULL || regForDisplayBackgroun.width()<0 || regForDisplayBackgroun.height()<=0)
		{
			printf("Error in RenderDrawing thread : failed to setup Background.(Provider=%d displayReg L=%f R=%f T=%f B=%f)\n",
				dcp, regForDisplayBackgroun.left, regForDisplayBackgroun.right, regForDisplayBackgroun.top, regForDisplayBackgroun.bottom);
		}
		else
		{
			render->setupBackground(dcp, regForDisplayBackgroun);
		}
	}

	m_render = render;

	BigViewportPartition* vpp = NULL;
	DisplayElement* de = NULL;
	LARGE_INTEGER freq;
	LARGE_INTEGER cur;
	LONGLONG lastTime = 0;
	LONGLONG nowTime = 0;
	QueryPerformanceFrequency(&freq);
	/////////////////用于从rendertarget中将数据拷贝出来////////////////////////
	UINT dstDataLen = width * height * 4;
	unsigned char* pdstData = (unsigned char*)malloc(dstDataLen);
	int dstWidth = 0, dstHeight = 0;
	int dstPixfmt = 0;
	int dstPitch = 0;
	std::ofstream dstOuput("output.rgb32", std::ios::out | std::ios::binary);
	//////////////////////////////////////////////////////////////////////////

	zRender::ElemDsplModel<zRender::BasicEffect>* pDsplModel = NULL;
	if (0 != zRender::CreateDsplModel<zRender::BasicEffect>(_T("D:\\代码黑洞\\3DRenderEngine\\DxRender\\FX\\DefaultVideo.fxo"), m_render, &pDsplModel) || NULL==pDsplModel)
	{
		return 0;
	}
	m_dsplModel = pDsplModel;
	int countForResize = 0;
	static const int MaxCountForResize = 300;
	while(m_isRunning)
	{
		++countForResize;
		if (countForResize % (MaxCountForResize * 2) == 0)
		{
			m_render->resize(1920, 1080);
		}
		else if (countForResize % MaxCountForResize == 0)
		{
			m_render->resize(800, 600);
		}
		QueryPerformanceCounter(&cur);
		nowTime = cur.QuadPart * 1000 / freq.QuadPart;
		if(lastTime!=0)
		{
			int interval = nowTime - lastTime;
			//printf("interval %d\n", interval);
		}
		lastTime = nowTime;
		if(WaitForSingleObject(m_timerHandle, INFINITE)==WAIT_FAILED)
			return -2;
		render->clear(0);
		//render->drawBackground();
		std::list<BigViewportPartition*>::iterator iter = m_vpPartitions.begin();
		for(; iter!=m_vpPartitions.end(); iter++)
		{
			vpp = *iter;
			if(NULL==vpp)
				continue;
			if(!vpp->isValid())	//不再可用，可能是BigViewport已被释放或者BigViewport、BigWindow已经Move到别的RenderDrawing
			{
				if(vpp->isNeedRelease())
				{
					*iter = NULL;
					de = vpp->getAttachedDisplayElement();
					m_render->releaseDisplayElement(&de);
					delete vpp;
				}
				continue;
			}
			de = vpp->getAttachedDisplayElement();
			if (de->getDsplModel() == NULL && NULL != m_dsplModel)
			{
				de->setDsplModel(m_dsplModel);
			}
			drawBigViewportPartition(render, vpp);
		}
		render->present(0);
		/*//将RenderTarget中的内容保存到文件中
		//zRender::TextureResource* snpRes = m_render->getSnapshot(TEXTURE_USAGE_STAGE, false, true);
		zRender::TextureResource* snpRes = m_render->getSnapshot(TEXTURE_USAGE_DEFAULT, true, true);
		if (snpRes != NULL)
		{
			//dump Texture to file
			//snpRes->dumpToFile(_T("dx_render_snapshot.rgb"));
			delete snpRes;
		}
		//if(0==m_render->getSnapshot(pdstData, dstDataLen, dstWidth, dstHeight, dstPixfmt, dstPitch))
		//{
		//	int len = dstPitch * dstHeight;
		//	dstOuput.write((char*)pdstData, len);
		//}*/
		Sleep(10);
	}
	free(pdstData);
/*
	float left, right;
	if(m_ltPointX==0.0)
	{
		left = 0.5;
		right = m_rbPointX;
	}
	else
	{
		left = m_ltPointX;
		right = 1.5;
	}
	zRender::DisplayElement* displayEle = render->createDisplayElement(zRender::RECT_f(0.5, 1.5, 1.25, 1.75));//left, right, m_ltPointY, m_rbPointY

	zRender::Vertex ver[4];
	ver[0].Pos = XMFLOAT3(0.0, 0.0, 0.0);
	ver[0].Tex = XMFLOAT2(0.0, 1.0);
	ver[1].Pos = XMFLOAT3(1.0, 0.0, 0.0);
	ver[1].Tex = XMFLOAT2(1.0, 1.0);
	ver[2].Pos = XMFLOAT3(0.0, 1.0, 0.0);
	ver[2].Tex = XMFLOAT2(0.0, 0.0);
	ver[3].Pos = XMFLOAT3(1.0, 1.0, 0.0);
	ver[3].Tex = XMFLOAT2(1.0, 0.0);
	UINT index[6] = {0, 2, 3, 0, 3, 1};
	zRender::VertexVector verVec(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	verVec.addVertexs(ver, 4, index, 6);
	displayEle->setVertex(&verVec);

	//RECT winrect = m_hwnd->getWindowRect();
	int picWidth = 720;//winrect.right - winrect.left;
	int picHeight = 576;//winrect.bottom - winrect.top;
	displayEle->setTexture(zRender::YUVFORMAT_PACKED_YUYV, picWidth, picHeight, NULL);


	while(m_isRunning)
	{
		render->draw(displayEle);
		render->present(0);
		Sleep(100);
	}
	*/
	return 0;
}

void RenderDrawing::drawBigViewportPartition(zRender::DxRender* render, BigViewportPartition* vpPartition)
{
	DisplayElement* de = vpPartition->getAttachedDisplayElement();
	if(NULL==de)
		return;
	de->createRenderResource();
	int retUpdate = vpPartition->update();
	if(0!=retUpdate) //不需更新
		return;
	render->draw(de);
}