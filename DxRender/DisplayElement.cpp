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

using namespace zRender;
#define LOG_TAG L"DxRender_DisplayElement"

DisplayElement::DisplayElement(DxRender* dxRender, ID3D11Device* d3dDevice, ID3D11DeviceContext* context)
	: m_dxRender(dxRender), m_device(d3dDevice), m_context(context)
	, m_IndexBuf(NULL), m_VertexBuf(NULL)
	, m_texture(NULL), m_isTextureUpdated(false)
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

//int DisplayElement::setTexture(PIXFormat pixfmt, int width, int height)
//{
//	if(NULL==m_device || NULL==m_dxRender)
//	{
//#ifdef _DEBUG
//		printf("DisplayElement obj have not be inited yet.\n");
//#endif
//		return -1;
//	}
//	if(pixfmt==PIXFMT_UNKNOW || width<=0 || height<=0/* || dataSrc==NULL*/)//fix me
//	{
//#ifdef _DEBUG
//		printf("Error in DisplayElement::setTexture : param invalid.(FMT=%d, W=%d, H=%d)\n",
//			pixfmt, width, height);
//#endif
//		return -2;
//	}
//	
//	if(PixelByteCount[pixfmt]!=PixelByteCount[m_TexFmt] ||
//		width!=m_TexWidth || height!=m_TexHeight)
//		m_isTextureUpdated = true;
//	m_TexFmt = pixfmt;
//	m_TexHeight = height;
//	m_TexWidth = width;
//	return 0;
//}

int DisplayElement::setTextureDataSource(TextureDataSource* dataSrc, const RECT_f& textureReg)
{
	if (m_MetaData == nullptr)
		return DXRENDER_RESULT_NOT_INIT_YET;
	if (dataSrc == m_MetaData->getTextureDataSource())
		return DXRENDER_RESULT_OK;
	m_MetaData->setTextureDataSource(dataSrc);
	m_isTextureUpdated = true;
	return DXRENDER_RESULT_OK;
	/*
	if(dataSrc==NULL)//如果dataSrc为NULL，则忽略对textureReg参数的检查
	{
		m_TexDataSrc = dataSrc;
		return 0;
	}
	if(textureReg.left<0 || textureReg.left>=1 || textureReg.top<0 || textureReg.top>=1
		|| textureReg.width() <= 0 || textureReg.height()<=0 || textureReg.right>1 ||textureReg.bottom>1)
	{
#ifdef _DEBUG
		printf("Error in DisplayElement::setTextureDataSource : param [textureReg] is invalid.(L=%f, R=%f, T=%f, B=%f)\n",
			textureReg.left, textureReg.right, textureReg.top, textureReg.bottom);
#endif
		return -1;
	}
	m_TexDataSrc = dataSrc;
	m_TexEffectiveReg = textureReg;
	return 0;*/
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
		if(0!=ret)
			return -3;
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
	return 0;
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
/*	if(m_texture!=NULL)	return -1;
	IRawFrameTexture* texTmp = m_dxRender->createTexture(m_TexFmt, m_TexWidth, m_TexHeight);
	if(texTmp==NULL)
		return -2;
	m_texture = texTmp;*/
	if (nullptr == m_MetaData)
		return DXRENDER_RESULT_NOT_INIT_YET;

	TextureDataSource* dataSrc = m_MetaData->getTextureDataSource();
	if (dataSrc != nullptr)
	{
		return createTextureResource(dataSrc);
	}
	return DXRENDER_RESULT_OK;
}

int DisplayElement::createTextureResource(TextureDataSource* dataSrc)
{
	int count = dataSrc->getTextureCount();
	if (count <= 0)  // 无需Texture资源
		return 0;
	TextureSourceDesc desc;
	for (int i = 0; i < count; i++)
	{
		if (DXRENDER_RESULT_OK == dataSrc->getTextureSourceDesc(i, &desc))
		{
			TextureResource* texRes = nullptr;
			int ret = m_dxRender->createTextureResource(desc, &texRes);
			if (DXRENDER_RESULT_OK != ret || nullptr == texRes)
			{
				log_e(_T("DisplayElement"), _T("Create TextureResource failed"));
				continue;
			}
			m_textureRes.push_back(TextureResContext(texRes));
		}
	}

	return m_textureRes.size()==count ? DXRENDER_RESULT_OK : DXRENDER_RESULT_CREATE_BUF_FAILED;
}

void DisplayElement::releaseTextureResource()
{
	if(m_texture)
	{
		m_texture->destroy();
		delete m_texture;
		m_texture = NULL;
	}
	for (size_t i = 0; i < m_textureRes.size(); i++)
	{
		m_dxRender->releaseTextureResource(&m_textureRes[i].texture);
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

int DisplayElement::updateTexture(int& identify)
{
	if(m_context==NULL)
	{
#ifdef _DEBUG
		printf("Error in DisplayElement::updateTexture : m_context is NULL.\n");
#endif
		return -1003;
	}
	if(!this->isValid())
	{
#ifdef _DEBUG
		printf("DisplayElement Not init.Resource have not been created.\n");
#endif
		return -1001;
	}
	if(m_texture==NULL || m_TexDataSrc==NULL)
	{
		return -1002;
	}
	int tex2dCount = 4;
	ID3D11Texture2D* tex2d[4] = {NULL, NULL, NULL, NULL};
	m_texture->getTexture(tex2d, tex2dCount);
	if(tex2dCount<=0)
		return -1004;

 	RECT effectReg;
 	//zRender::SharedTexture* shTex = m_TexDataSrc->getSharedTexture(effectReg, identify);
	int dataLen, width, height, yPitch, uPitch, vPitch;
	PIXFormat pixfmt;
	unsigned char* pData = m_TexDataSrc->getData(dataLen, yPitch, uPitch, vPitch, width, height, pixfmt, effectReg, identify);
	int effectRegWidth = effectReg.right-effectReg.left;
	int effectRegHeight = effectReg.bottom-effectReg.top;
	if(pData==NULL || dataLen<=0 || width<=0 || height<=0 
		|| yPitch<=0 || pixfmt!=m_texture->getPixelFormat() || effectReg.left<0 || effectReg.top<0
		|| effectRegWidth<=0 || effectRegHeight<=0 || effectReg.bottom>height || effectReg.right>width)
		return -1005;

	RECT copyedReg;
	copyedReg.left = (LONG)(effectReg.left + (effectRegWidth * m_TexEffectiveReg.left + 0.5));
	copyedReg.right = (LONG)(effectReg.left + (effectRegWidth * m_TexEffectiveReg.right + 0.5));
	copyedReg.top = (LONG)(effectReg.top + (effectRegHeight * m_TexEffectiveReg.top + 0.5));
	copyedReg.bottom = (LONG)(effectReg.top + (effectRegHeight * m_TexEffectiveReg.bottom + 0.5));
	return m_TexDataSrc->copyDataToTexture(RECT_f(0, 1, 0, 1), pData, yPitch, height, identify);
	return m_texture->update(pData, dataLen, yPitch, uPitch, vPitch, width, height, copyedReg, m_context);
	return -1005;
	//return -1005;
 	//else
 	//{
 	//	return m_texture->update(shTex, effectReg, m_context);
 	//}
	////////////////////////////////////////////////////////////////////////////////
// 	D3D11_MAPPED_SUBRESOURCE mappedRes;
// 	ZeroMemory(&mappedRes, sizeof(mappedRes));
// 	D3D11_TEXTURE2D_DESC texDesc;
// 	tex2d[0]->GetDesc(&texDesc);
// 	m_context->Map(tex2d[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes);
// 	unsigned char* pDes = (unsigned char*)mappedRes.pData;
// 	m_TexDataSrc->copyDataToTexture(m_TexEffectiveReg, pDes, mappedRes.RowPitch, m_TexHeight, identify);
// 	m_context->Unmap(tex2d[0], 0);
// 	return 0;
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
			textureDataSrc->updateTextures(textureCtx.texture, i, textureCtx.identifyForData);
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

int zRender::DisplayElement::openSharedTexture(IRawFrameTexture * sharedTexture)
{
	if (sharedTexture == NULL)
		return -1;
	if (m_texture)
	{
		//releaseTexTureResource();
		return 0;
	}
	m_texture = m_dxRender->openSharedTexture(sharedTexture);
	if (NULL == m_texture)
		return -2;
	return 0;
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
	return m_DrawingContext->apply(m_dxRender, this->getTexture(), this->getVertexBuffer(), this->getIndexBuffer(), m_MetaData);
}
