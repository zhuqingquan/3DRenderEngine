#include "DisplayElement.h"
#include "DxRender_D3D11.h"
#include <assert.h>
#include "DXLogger.h"
#include "SharedFrameTexture.h"
#include "ElemDsplModel.h"
#include "ConstDefine.h"
#include "RectangleDataCtxInitializer.h"
#include "ElementDrawingContext.h"
#include "ElementMetaData.h"
#include "inc/TextureResource.h"
#include <TextureResourceFactory.h>
#include <TextureFasterCopyHelper.h>

using namespace zRender;
#define LOG_TAG L"DxRender_DisplayElement"

DisplayElement::DisplayElement(DxRender* dxRender, ID3D11Device* d3dDevice, ID3D11DeviceContext* context)
	: m_dxRender(dxRender), m_device(d3dDevice), m_context(context)
	, m_IndexBuf(NULL), m_VertexBuf(NULL)
	, m_isTextureUpdated(false)
	, m_isVertexInfoUpdated(false)
	, m_TexFmt(PIXFMT_UNKNOW), m_TexWidth(0), m_TexHeight(0), m_TexDataSrc(NULL)
	, m_DataCtxInitializer(new RectangleDataCtxInitializer())
	, m_DrawingContext(nullptr), m_MetaData(nullptr)
{
	if (0 != m_DataCtxInitializer->init(dxRender))
	{
		log_e(LOG_TAG, _T("Init Data context failed."));
	}
	else
	{
		m_DrawingContext = m_DataCtxInitializer->GetDrawingContext();
		m_MetaData = m_DataCtxInitializer->GetMetaData();
		m_isVertexInfoUpdated = true;
	}
}

DisplayElement::~DisplayElement()
{
	if (m_DataCtxInitializer != nullptr)
	{
		m_DataCtxInitializer->deinit();
		delete m_DataCtxInitializer;
		m_DataCtxInitializer = nullptr;
	}
	releaseRenderResource();
	m_applyingTexture.clear();
}

int DisplayElement::setDisplayRegion(const RECT_f& displayReg, float zIndex)
{
	if (m_DrawingContext != nullptr)
	{
		return m_DrawingContext->setDisplayRegion(displayReg, zIndex);
	}
	return DXRENDER_RESULT_NOT_INIT_YET;
}

int DisplayElement::setZIndex(float zIndex)
{
	if (nullptr == m_DrawingContext)
		return DXRENDER_RESULT_NOT_INIT_YET;
	return m_DrawingContext->setZIndex(zIndex);
}

float zRender::DisplayElement::getZIndex() const
{
	if (nullptr == m_DrawingContext)
		return -1.0f;
	return m_DrawingContext->getZIndex();
}

int DisplayElement::setTextureDataSource(TextureDataSource* dataSrc, const RECT_f& textureReg)
{
	if (m_MetaData == nullptr)
		return DXRENDER_RESULT_NOT_INIT_YET;
	if (dataSrc == m_MetaData->getTextureDataSource())
		return DXRENDER_RESULT_OK;
	m_MetaData->setTextureDataSource(dataSrc);
	m_isTextureUpdated = true;
	return DXRENDER_RESULT_OK;
}

int DisplayElement::createRenderResource()
{
	if(NULL==m_device || NULL==m_dxRender)
	{
		log_e(_T("DisplayElement"), _T("DisplayElement", "DisplayElement obj have not be inited yet.\n"));
		return DXRENDER_RESULT_NOT_INIT_YET;
	}
	if(m_isTextureUpdated)		//创建Texture
	{
		releaseTextureResource();
		int ret = createTextureResource();
		if(DXRENDER_RESULT_OK!=ret)
			return ret;
		m_isTextureUpdated = false;
	}
	if(m_isVertexInfoUpdated)	//创建Vertex、Index Buffer
	{
		int ret = releaseIndexBuffer();
		if(0!=ret)
			return -4;
		ret = createIndexBuffer();
		if(0!=ret)
			return -5;
		ret = releaseVertexBuffer();
		if(0!=ret)
			return -6;
		ret = createVertexBuffer();
		if(0!=ret)
			return -7;
		m_isVertexInfoUpdated = false;
	}
	return DXRENDER_RESULT_OK;
}

int DisplayElement::releaseRenderResource()
{
	releaseVertexBuffer();
	releaseIndexBuffer();
	releaseTextureResource();
	return 0;
}

int DisplayElement::createTextureResource()
{
	if (nullptr == m_MetaData)
		return DXRENDER_RESULT_NOT_INIT_YET;

	TextureDataSource* dataSrc = m_MetaData->getTextureDataSource();
	if (dataSrc != nullptr)
	{
		return createTextureResource(dataSrc);
	}
	return DXRENDER_RESULT_METADATA_INVALID;
}

int DisplayElement::createTextureResource(TextureDataSource* dataSrc)
{
	int count = dataSrc->getTextureCount();
	if (count <= 0)  // 无需Texture资源
		return DXRENDER_RESULT_OK;
	TextureSourceDesc desc;
	for (int i = 0; i < count; i++)
	{
		if (DXRENDER_RESULT_OK == dataSrc->getTextureSourceDesc(i, &desc))
		{
			TextureFasterCopyHelper* textureHelper = new TextureFasterCopyHelper();
			int ret = textureHelper->create(m_dxRender, desc);
			if (DXRENDER_RESULT_OK != ret)
			{
				log_e(_T("DisplayElement"), _T("Create TextureResource failed"));
				continue;
			}
			ret = textureHelper->getTextureForDraw()->createResourceView();
			if (DXRENDER_RESULT_OK != ret)
			{
				log_e(_T("DisplayElement"), _T("Create Texture View failed."));
				continue;
			}
			m_textureRes.push_back(TextureResContext(textureHelper->getTextureForDraw(), textureHelper));
		}
	}

	setupApplyingTextureList();
	return m_textureRes.size()==count ? DXRENDER_RESULT_OK : DXRENDER_RESULT_CREATE_BUF_FAILED;
}

void DisplayElement::setupApplyingTextureList()
{
	m_applyingTexture.clear();
	for (size_t i = 0; i < m_textureRes.size(); i++)
	{
		TextureResContext& textureCtx = m_textureRes[i];
		if (textureCtx.texture == nullptr)
			continue;
		m_applyingTexture.push_back(textureCtx.texture);
	}
}

void DisplayElement::releaseTextureResource()
{
	for (size_t i = 0; i < m_textureRes.size(); i++)
	{
		m_textureRes[i].texture->releaseResourceView();
		m_textureRes[i].textureHelper->release();
		delete m_textureRes[i].textureHelper;
	}
	m_textureRes.clear();
}

int DisplayElement::createVertexBuffer()
{
	if (m_MetaData == nullptr)
		return DXRENDER_RESULT_NOT_INIT_YET;
	if(m_VertexBuf)	return DXRENDER_RESULT_FUNC_REENTRY_INVALID;
	VertexVector* verVec = m_MetaData->getVertexData();
	int initDataLen = 0;
	const unsigned char* initData = verVec->getVertexData(initDataLen);
	if(NULL==initData || initDataLen<=0)
		return DXRENDER_RESULT_METADATA_INVALID;
	ID3D11Buffer* vb = m_dxRender->createVertexBuffer(initDataLen, initData, initDataLen);
	if(vb==NULL)
		return DXRENDER_RESULT_CREATE_BUF_FAILED;
	m_VertexBuf = vb;
	return DXRENDER_RESULT_OK;
}

int DisplayElement::releaseVertexBuffer()
{
	return m_dxRender->releaseBuffer(&m_VertexBuf);
}

int DisplayElement::createIndexBuffer()
{
	if (m_MetaData == nullptr)
		return DXRENDER_RESULT_NOT_INIT_YET;
	if(m_IndexBuf)	return DXRENDER_RESULT_FUNC_REENTRY_INVALID;
	VertexVector* verVec = m_MetaData->getVertexData();
	int initDataLen = 0;
	const unsigned char* initData = verVec->getIndexData(initDataLen);
	if(NULL==initData || initDataLen<=0)
		return DXRENDER_RESULT_METADATA_INVALID;
	ID3D11Buffer* ib = m_dxRender->createIndexBuffer(initDataLen, initData, initDataLen);
	if(ib==NULL)
		return DXRENDER_RESULT_CREATE_BUF_FAILED;
	m_IndexBuf = ib;
	return DXRENDER_RESULT_OK;
}

int DisplayElement::releaseIndexBuffer()
{
	return m_dxRender->releaseBuffer(&m_IndexBuf);
}

bool DisplayElement::isValid() const
{
	if(m_VertexBuf==NULL || m_IndexBuf==NULL)
		return false;
	return true;
}

int zRender::DisplayElement::updateTexture()
{
	if (m_context == nullptr || !isValid())
		return DXRENDER_RESULT_NOT_INIT_YET;
	if(m_textureRes.size()<=0)
		return DXRENDER_RESULT_OK;
	TextureDataSource* textureDataSrc = m_MetaData->getTextureDataSource();
	if (textureDataSrc==nullptr || m_textureRes.size() != textureDataSrc->getTextureCount())
		return DXRENDER_RESULT_CREATE_BUF_FAILED;
	for (size_t i = 0; i < m_textureRes.size(); i++)
	{
		TextureResContext& textureCtx = m_textureRes[i];
		if (textureCtx.texture == nullptr)
			continue;
		if (textureDataSrc->isTextureUpdated(i, textureCtx.identifyForData))
		{
			int ret = textureDataSrc->updateTextures(textureCtx.textureHelper->getTextureForUpdate(), i, textureCtx.identifyForData);
			if (ret != DXRENDER_RESULT_OK)
				continue;
			textureCtx.textureHelper->getTextureForDraw()->copyResource(textureCtx.textureHelper->getTextureForUpdate());
		}
	}
	return DXRENDER_RESULT_OK;
}

void zRender::DisplayElement::setAlpha( float alpha )
{
	if (m_DrawingContext != nullptr)
		m_DrawingContext->setAlpha(alpha);
}

float zRender::DisplayElement::getAlpha() const
{
	return m_DrawingContext != NULL ? m_DrawingContext->getAlpha() : 1.0f;
}

void zRender::DisplayElement::enableTransparent( bool enable )
{
	if (m_DrawingContext != nullptr)
		m_DrawingContext->enableTransparent(enable);
}

bool zRender::DisplayElement::isEnableTransparent() const
{
	return m_DrawingContext ? m_DrawingContext->isEnableTransparent() : false;
}

int zRender::DisplayElement::draw()
{
	//如果没有显示模型，则该Element无法完成显示，因为根本不知道该如何显示
	if (m_DrawingContext==nullptr)
		return DXRENDER_RESULT_NOT_INIT_YET;
	if(0!=this->createRenderResource() || !this->isValid() )
	{
		TCHAR errmsg[512] = {0};
		swprintf_s(errmsg, 512, L"Error in DxRender_D3D11::draw : DisplayElement is invalid.");
		log_e(LOG_TAG, errmsg);
		return -4;
	}

	return m_DrawingContext->apply(m_dxRender, m_applyingTexture.size()<=0 ? nullptr : &m_applyingTexture[0], m_applyingTexture.size(), 
		this->getVertexBuffer(), this->getIndexBuffer(), m_MetaData);
}
