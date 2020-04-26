#include "BigViewportPartition.h"
#include <assert.h>
#include "DisplayElement.h"
#include "BigView.h"

using namespace SOA::Mirror::Render;
using namespace zRender;

BigViewportPartition::BigViewportPartition(const zRender::RECT_f& regOfBigScreen, const zRender::RECT_f& regOfBigViewport, RenderDrawing* rd)
	: m_regOfBigScreen(regOfBigScreen), m_regOfBigViewport(regOfBigViewport)
	, m_renderDrawing(rd), m_attachedDE(NULL), m_attachedView(NULL)
	, m_cttProvider(NULL), m_curDrawedVertexIdentify(0), m_curDrawedTextureIdentify(0)
	, m_ZIndex(0)
{
}

BigViewportPartition::~BigViewportPartition()
{
	
}

int BigViewportPartition::attachDisplayElement(zRender::DisplayElement* de)
{
	assert(de);
	m_attachedDE = de;
	return 0;
}

zRender::DisplayElement* BigViewportPartition::getAttachedDisplayElement() const
{
	return m_attachedDE;
}

int BigViewportPartition::attachBigView(BigView* view)
{
	if(NULL==view)
	{
#ifdef _DEBUG
		printf("Error in BigViewportPartition::attachBigView : param is NULL.\n");
#endif
		return -1;
	}
	IDisplayContentProvider* cttProvider = view->applyAuthorization(this);
	if(cttProvider==NULL)
	{
#ifdef _DEBUG
		printf("Error in BigViewportPartition::attachBigView : faile to apply authorization from view.\n");
#endif
		return -2;
	}

	m_cttProvider = cttProvider;
	update();
	m_attachedView = view;
	return 0;
}

int BigViewportPartition::disattachView()
{
	if(m_cttProvider)
	{
		m_cttProvider = NULL;
		m_curDrawedTextureIdentify = 0;
		m_curDrawedVertexIdentify = 0;
	}
	if(m_attachedView)
	{
		m_attachedView->releaseAutorization(this);
		m_attachedView = NULL;
	}
	m_attachedDE->setTextureDataSource(NULL, RECT_f());
	return 0;
}

BigView* BigViewportPartition::getAttachedView() const
{
	return m_attachedView;
}

RenderDrawing* BigViewportPartition::getRenderDrawing() const
{
	return m_renderDrawing;
}

int BigViewportPartition::move(const const zRender::RECT_f& regOfBigScreen, const zRender::RECT_f& regOfBigViewport)
{
	throw "Not implement";
	return -1;
}

int BigViewportPartition::update()
{
	if(m_cttProvider==NULL)
		return -1;
	updateVertex();
	updateTexture();
	return 0;
}

void BigViewportPartition::updateVertex()
{
	//VertexVector* vv = NULL;
	//int vvCount = 1;
	//if(m_cttProvider->isVertexUpdated(m_curDrawedVertexIdentify))
	//{
	//	if(0!=m_cttProvider->getVertexs(&vv, vvCount, m_curDrawedVertexIdentify))
	//	{
	//		return;
	//	}
	//	if(vv==NULL || vvCount<=0)
	//		return;
	//	for (int i = 0; i < vvCount; i++)
	//	{
	//		VertexVector* curVV = vv + i;
	//		m_attachedDE->setVertex(curVV);
	//	}
	//	//m_attachedDE->createRenderResource();
	//}
}

void BigViewportPartition::updateTexture()
{
	TextureDataSource* tds = m_cttProvider->getTextureDataSource();
	if(tds==NULL)
		return;
	if(tds->isUpdated(m_curDrawedTextureIdentify))
	{
		int ret = -1;
		int dataLen = 0;
		int pitch = 0;
		int uPitch = 0;
		int vPitch = 0;
		int width = 0;
		int height = 0;
		PIXFormat pixelFmt = PIXFMT_UNKNOW;
		ret = tds->getTextureProfile(m_regOfBigViewport, dataLen, pitch, uPitch, vPitch, width, height, pixelFmt);
		if(ret!=0 || dataLen==0 || pixelFmt==0 || width==0 || height==0)
			return;
		//m_attachedDE->setTexture(pixelFmt, width, height);
		zRender::TextureDataSource* texDataSrc = m_cttProvider->getTextureDataSource();
		zRender::IRawFrameTexture* rawTexture = texDataSrc->getTexture();
		m_attachedDE->openSharedTexture(rawTexture);
		if (pixelFmt == PIXFMT_A8R8G8B8 || PIXFMT_R8G8B8A8 == pixelFmt || PIXFMT_B8G8R8A8 == pixelFmt)
		{
			m_attachedDE->enableTransparent(true);
		}
		m_attachedDE->setTextureDataSource(tds, m_regOfBigViewport);
		m_attachedDE->createRenderResource();
		//m_attachedDE->updateTexture(m_curDrawedTextureIdentify);
		m_attachedDE->updateTexture();
	}
}

int BigViewportPartition::notifyToRelease()
{
	if(0==disattachView())
	{
		m_curDrawedTextureIdentify = -1;
		m_curDrawedVertexIdentify = -1;
		return 0;
	}
	return -1;
}

bool BigViewportPartition::isValid() const
{
	if(m_attachedDE==NULL || m_attachedView==NULL)
		return false;
	return true;
}

void BigViewportPartition::setZIndex(int zIndex)
{
	m_ZIndex = zIndex;
	if (m_attachedDE != NULL)
	{
		m_attachedDE->setZIndex(zIndex);
	}
}