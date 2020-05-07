#include "inc/SharedTextureSource.h"
#include "DxRender.h"

using namespace zRender;

SharedTextureSource::SharedTextureSource(DxRender * render)
	: m_dxrender(render)
//	, m_texShared(NULL), m_texStaging(NULL)
	, m_isUpdatedIdentify(0)
	, m_cacheData(NULL)
{
}

SharedTextureSource::~SharedTextureSource()
{
}

bool SharedTextureSource::isUpdated(int identify) const
{
	return m_isUpdatedIdentify>identify;
}

int SharedTextureSource::getTextureProfile(const RECT_f & textureReg, int & dataLen, int & yPitch, int & uPitch, int & vPitch, int & width, int & height, PIXFormat & pixelFmt)
{
/*	if (NULL == m_texStaging)
		return -1;
	int w_full = m_texStaging->getWidth();
	int h_full = m_texStaging->getHeight();
	pixelFmt = m_texStaging->getPixelFormat();
	if (w_full <= 0 || h_full <= 0)
		return -2;
	width = w_full * (textureReg.width());
	height = h_full * (textureReg.height());
	switch (pixelFmt)
	{
	case PIXFMT_A8R8G8B8:
	case PIXFMT_B8G8R8A8:
	case PIXFMT_B8G8R8X8:
	case PIXFMT_R8G8B8A8:
	case PIXFMT_R8G8B8X8:
		yPitch = width * 4;
		uPitch = 0;
		vPitch = 0;
		dataLen = yPitch * height;
		break;
	case PIXFMT_R8G8B8:
		yPitch = width * 3;
		uPitch = 0;
		yPitch = 0;
		dataLen = yPitch * height;
		break;
	case PIXFMT_NV12:
	case PIXFMT_YV12:
		yPitch = width;
		uPitch = width;
		vPitch = 0;
		dataLen = yPitch * height + uPitch * (height >> 1);
		break;
	case PIXFMT_YUV420P:
		yPitch = width;
		uPitch = width >> 1;
		vPitch = width >> 1;
		dataLen = yPitch * height + uPitch * (height >> 1) + vPitch * (height >> 1);
		break;
	case PIXFMT_YUY2:
		yPitch = width * 2;
		uPitch = 0;
		vPitch = 0;
		dataLen = yPitch * height;
		break;
	case PIXFMT_UNKNOW:
	default:
		return -3;
	}*/
	return 0;
}

//no use
unsigned char * SharedTextureSource::getData(int & dataLen, int & yPitch, int & uPitch, int & vPitch, int & width, int & height, PIXFormat & pixelFmt, RECT & effectReg, int & identify)
{
	if (m_cacheData == NULL)
		return NULL;/*
	dataLen = m_cache_pitch * m_cache_height;
	yPitch = m_cache_pitch;
	uPitch = 0;
	vPitch = 0;
	width = m_cache_width;
	height = m_cache_height;
	pixelFmt = m_texStaging->getPixelFormat();
	effectReg.left = 0;
	effectReg.top = 0;
	effectReg.right = width;
	effectReg.bottom = height;*/
	return m_cacheData;
}

bool zRender::SharedTextureSource::isTextureUpdated(int index, unsigned int identify)
{
	if (index < 0 || index>1)
		return false;
	return m_isUpdatedIdentify > identify;;
}

//no use
SharedTexture * SharedTextureSource::getSharedTexture(RECT & effectReg, int & identify)
{
	return NULL;
}

int SharedTextureSource::copyDataToTexture(const RECT_f & textureReg, unsigned char * dstTextureData, int pitch, int height, int & identify)
{
	/*if (NULL == m_texStaging)
	{
		return -1;
	}
	int w_full = m_texStaging->getWidth();
	int h_full = m_texStaging->getHeight();
	RECT updateReg = { 0 };
	updateReg.left = textureReg.left * w_full;
	updateReg.right = textureReg.right * w_full;
	updateReg.top = textureReg.top * h_full;
	updateReg.bottom = textureReg.bottom * h_full;
	if (0 != m_texStaging->update(dstTextureData, pitch * height, pitch, 0, 0, w_full, height, updateReg, NULL))
	{
		return -2;
	}
	if (0 != m_texShared->copyTexture(m_texStaging))
	{
		return -3;
	}
	m_isUpdatedIdentify++;
	identify = m_isUpdatedIdentify;*/
	return 0;
}

int SharedTextureSource::createTexture(PIXFormat pixfmt, int w, int h)
{/*
	if (m_dxrender == NULL)	return -1;
	IRawFrameTexture* sharedTex = m_dxrender->createTexture(pixfmt, w, h, TEXTURE_USAGE_DEFAULT, true, NULL, 0, 0);
	if (NULL == sharedTex)
	{
		return -2;
	}
	m_texShared = sharedTex;
	IRawFrameTexture* stagingTex = m_dxrender->createTexture(pixfmt, w, h, TEXTURE_USAGE_STAGE, false, NULL, 0, 0);
	if (NULL == stagingTex)
	{
		return -3;
	}
	m_texStaging = stagingTex;
*/
	m_srcDesc.width = w;
	m_srcDesc.height = h;
	m_srcDesc.pixelFmt = pixfmt;
	m_isUpdatedIdentify++;
	return 0;
}

void SharedTextureSource::releaseTexture()
{/*
	if (NULL == m_dxrender)
		return;
	if (m_texStaging)
	{
		m_dxrender->releaseTexture(&m_texStaging);
	}
	if (m_texShared)
	{
		m_dxrender->releaseTexture(&m_texShared);
	}*/
}

void zRender::SharedTextureSource::cacheData(const RECT_f & textureReg, unsigned char * dstTextureData, int pitch, int width, int height)
{
	m_cacheData = dstTextureData;
	m_cache_width = width;
	m_cache_height = height;
	m_cache_pitch = pitch;
	m_cache_textureReg = textureReg;
	m_srcDesc.buffers[0] = (char*)dstTextureData;
	m_srcDesc.pitchs[0] = pitch;
	m_srcDesc.width = width;
	m_srcDesc.height = height;
	m_isUpdatedIdentify++;
}
