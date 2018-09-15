// DxRender.cpp : 定义 DLL 应用程序的导出函数。
//

#include "DxRender.h"
#include "DxRender_D3D11.h"
#include "BackgroundDisplayComponent.h"
#include <assert.h>

using namespace zRender;

DxRender::DxRender()
	: m_renderImp(new DxRender_D3D11())
	, m_background(NULL)
{
}

DxRender::~DxRender()
{
	cancleBackground();

	delete m_renderImp;
	m_renderImp = NULL;
}

int DxRender::setVisibleRegion(const RECT_f& visibleReg)
{
	return m_renderImp->setVisibleRegion(visibleReg);
}

RECT_f DxRender::getVisibleRegion() const
{
	return m_renderImp->getVisibleREgion();
}

int DxRender::init(HWND hWnd, const wchar_t* effectFileName, bool isEnable4XMSAA /*= false*/, bool isSDICompatible/* = false*/)
{
	return m_renderImp->init(hWnd, effectFileName, isEnable4XMSAA, isSDICompatible);
}

int zRender::DxRender::init(HMONITOR hmonitor)
{
	return m_renderImp->init(hmonitor);
}

void DxRender::deinit()
{
	m_renderImp->deinit();
}

DisplayElement* DxRender::createDisplayElement(const RECT_f& displayReg, int zIndex)
{
	return m_renderImp->createDisplayElement(displayReg, zIndex);
}

int DxRender::releaseDisplayElement(DisplayElement** displayElement)
{
	return m_renderImp->releaseDisplayElement(displayElement);
}

IRawFrameTexture * DxRender::createTexture(PIXFormat pixfmt, int width, int height, TEXTURE_USAGE usage, bool bShared, unsigned char * initData, int dataLen, int pitch)
{
	return m_renderImp->createTexture(pixfmt, width, height, usage, bShared, initData, dataLen, pitch);
}

IRawFrameTexture * zRender::DxRender::openSharedTexture(IRawFrameTexture * sharedTexture)
{
	return m_renderImp->openSharedTexture(sharedTexture);
}

void DxRender::releaseTexture(IRawFrameTexture ** rawFrameTexture)
{
	m_renderImp->releaseTexture(rawFrameTexture);
}

int DxRender::draw(DisplayElement* displayElem)
{
	return m_renderImp->draw(displayElem);
}

int DxRender::present(int type)
{
	return m_renderImp->present(type);
}

int DxRender::clear(DWORD color)
{
	return m_renderImp->clear(color);
}

int DxRender::setupBackground(IDisplayContentProvider* contentProvider, const RECT_f& displayReg)
{
	if(m_background)
	{
		printf("Error in DxRender::setupBackground : background had setup allready.\n");
		return -1;
	}
	if(NULL==contentProvider || displayReg.width()<=0 || displayReg.height()<=0 
		|| !displayReg.isIntersect(m_renderImp->getVisibleREgion()))
	{
		printf("Error in DxRender::setupBackground : param invalid.(Provider=%d Reg.L=%f Reg.R=%f Reg.T=%f Reg.B=%f)\n",
			(int)contentProvider, displayReg.left, displayReg.right, displayReg.top, displayReg.bottom);
		return -2;
	}
	TextureDataSource* texDataSrc = contentProvider->getTextureDataSource();
	VertexVector* vvs[4] = {NULL};
	int vvsCount = 4;
	int vvIdt = 0;
	int ret = contentProvider->getVertexs(vvs, vvsCount, vvIdt);
	if(texDataSrc==NULL || ret!=0 || vvsCount==0)
	{
		printf("Error in DxRender::setupBackground : Content Provider invalid.(TexData=%d ret=%d vvsCount=%d)\n",
			(int)texDataSrc, ret, vvsCount);
		return -4;
	}

	DisplayElement* de = m_renderImp->createDisplayElement(displayReg, (int)(RANGE_OF_ZINDEX_MAX-180));
	assert(de);
	if(NULL==de)
	{
		printf("Error in DxRender::setupBackground : failed to create display element.\n");
		return -3;
	}
	de->setVertex(vvs[0]);
	int texIdt = 0;
	if(texDataSrc->isUpdated(texIdt))
	{
		ret = -1;
		int dataLen = 0;
		int ypitch = 0;
		int upitch = 0;
		int vpitch = 0;
		int width = 0;
		int height = 0;
		PIXFormat pixelFmt = PIXFMT_UNKNOW;
		RECT_f texEffectReg(0.0, 1.0, 0.0, 1.0);
		ret = texDataSrc->getTextureProfile(texEffectReg, dataLen, ypitch, upitch, vpitch, width, height, pixelFmt);
		if(ret!=0 || dataLen==0 || pixelFmt==0 || width==0 || height==0)
		{
			m_renderImp->releaseDisplayElement(&de);
			printf("Error in DxRender::setupBackground : get texture's profile failed.\n");			
			return -5;
		}
		de->setTexture(pixelFmt, width, height);
		de->setTextureDataSource(texDataSrc, texEffectReg);
		de->createRenderResource();
		int texUpdateIdt = 0;
		de->updateTexture(texUpdateIdt);
	}

	BackgroundComponent* bgCom = new BackgroundComponent(contentProvider, de);
	assert(bgCom);
	//if(0!=m_renderImp->setupBackground(bgCom))
	//{
	//	m_renderImp->releaseDisplayElement(&de);
	//	delete bgCom;
	//	printf("Error in DxRender::setupBackground : setup failed.\n");
	//	return -6;
	//}

	m_background = bgCom;
	return 0;
}

int DxRender::cancleBackground()
{
	if(m_background)
	{
		DisplayElement* de = m_background->getDisplayElement();
		delete de;
		delete m_background;
		m_background = NULL;
	}
	return 0;
}

int DxRender::drawBackground()
{
	if(m_background==NULL)
	{
		printf("Error in DxRender::drawBackground : background Not setup.\n");
		return -1;
	}
	int ret = m_renderImp->draw(m_background->getDisplayElement());
	if(0!=ret)
	{
		printf("Error in DxRender::drawBackground : draw failed.(ErrorCode=%d)\n", ret);
		return -2;
	}
	return 0;
}

int zRender::DxRender::lockBackbufferHDC( BOOL Discard, HDC* outHDC )
{
	return m_renderImp->lockBackbufferHDC(Discard, outHDC);
}

int zRender::DxRender::unlockBackbufferHDC( HDC hdc )
{
	return m_renderImp->unlockBackbufferHDC(hdc);
}

TextureResource * zRender::DxRender::getSnapshot(TEXTURE_USAGE usage, bool bShared, bool fromOffscreenTexture)
{
	return m_renderImp->getSnapshot(usage, bShared,fromOffscreenTexture);
}

void* zRender::DxRender::getDevice() const
{
	return m_renderImp->getDevice();
}

int zRender::DxRender::getWidth()
{
	return m_renderImp->getWidth();
}

int zRender::DxRender::getHeight()
{
	return m_renderImp->getHeight();
}

int zRender::DxRender::getSnapshot( unsigned char* pData, UINT& datalen, int& w, int& h, int& pixfmt, int& pitch )
{
	return m_renderImp->getSnapshot(pData, datalen, w, h, pixfmt, pitch);
}

int zRender::DxRender::getSnapshot(SharedTexture** outSharedTexture)
{
	return m_renderImp->getSnapshot(outSharedTexture);
}

int zRender::DxRender::resize( int new_width, int new_height )
{
	return m_renderImp->resize(new_width, new_height);
}

float zRender::DxRender::getAspectRatio() const
{
	return m_renderImp->getAspectRatio();
}

const XMFLOAT4X4& zRender::DxRender::getWorldBaseTransformMatrix() const
{
	return m_renderImp->getWorldBaseTransformMatrix();
}

const XMFLOAT4X4& zRender::DxRender::getViewTransformMatrix() const
{
	return m_renderImp->getViewTransformMatrix();
}

const XMFLOAT4X4& zRender::DxRender::getProjectionTransformMatrix() const
{
	return m_renderImp->getProjectionTransformMatrix();
}

int zRender::DxRender::createOffscreenRenderTarget(int width, int height)
{
	return m_renderImp->createOffscreenRenderTarget(width, height);
}

void zRender::DxRender::releaseOffscreenRenderTarget()
{
	m_renderImp->releaseOffscreenRenderTarget();
}

int zRender::DxRender::createSharedTexture(SharedTexture** ppSharedTex, PIXFormat pixfmt)
{
	return m_renderImp->createSharedTexture(ppSharedTex, pixfmt);
}

int zRender::DxRender::releaseSharedTexture(SharedTexture** ppSharedTex)
{
	return m_renderImp->releaseSharedTexture(ppSharedTex);
}

int zRender::DxRender::createTextureResource(TextureResource ** ppOutTexRes, int width, int height, DXGI_FORMAT dxgiFmt, TEXTURE_USAGE usage, bool bShared, const char * initData, int dataLen, int pitch)
{
	return m_renderImp->createTextureResource(ppOutTexRes, width, height, dxgiFmt, usage, bShared, initData, dataLen, pitch);
}

int zRender::DxRender::openSharedTextureResource(TextureResource ** ppOutTexRes, HANDLE hSharedRes)
{
	return m_renderImp->openSharedTextureResource(ppOutTexRes, hSharedRes);
}

void zRender::DxRender::releaseTextureResource(TextureResource ** ppOutTexRes)
{
	m_renderImp->releaseTextureResource(ppOutTexRes);
}
