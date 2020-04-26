#include "inc/RawFrameTextureBase.h"
#include "inc/TextureResource.h"

using namespace zRender;

RawFrameTextureBase::RawFrameTextureBase(PIXFormat pixfmt)
	: IRawFrameTexture(pixfmt)
	, m_textureCount(0)
	, m_textureArray(NULL)
{
	initTextureArray(pixfmt);
}

RawFrameTextureBase::~RawFrameTextureBase()
{
	releaseTextureArray();
}

/*virtual*/ int RawFrameTextureBase::openSharedTexture(ID3D11Device* device, IRawFrameTexture* sharedTexture)
{
	if (NULL == sharedTexture)	return -1;
	if (sharedTexture->getPixelFormat() != m_pixfmt)	return -2;
	int texCount = 8;
	TextureResource* texArray[8] = { NULL };
	if (0 != sharedTexture->getTextureResources(texArray, texCount))
	{
		return -2;
	}
	HANDLE sharedHandle[8] = { INVALID_HANDLE_VALUE };
	for (size_t i = 0; i < texCount; i++)
	{
		TextureResource* sharedTexRes = texArray[i];
		HANDLE hRes = INVALID_HANDLE_VALUE;
		if (!sharedTexRes || INVALID_HANDLE_VALUE == (hRes=sharedTexRes->getSharedHandle()))
			return -3;
		sharedHandle[i] = hRes;
	}
	for (size_t i = 0; i < texCount; i++)
	{
		TextureResource* newTexRes = new TextureResource();
		if (0 != newTexRes->open(device, sharedHandle[i]))
		{
			//fixme 如果单个TextureResource的open失败是否要删除之前所有的
			delete newTexRes;
			return -4;
		}
		else
		{
			m_textureArray[i] = newTexRes;
		}
	}
	return 0;
}

/*virtual*/ int RawFrameTextureBase::getTextureResources(TextureResource** outTexs, int& texsCount) const
{
	if (texsCount <= 0 || outTexs == NULL)
		return -1;
	if (texsCount < m_textureCount)
		return -2;
	for (size_t i = 0; i < m_textureCount; i++)
	{
		outTexs[i] = m_textureArray[i];
	}
	texsCount = m_textureCount;
	return 0;
}

int zRender::RawFrameTextureBase::copyTexture(const IRawFrameTexture * srcTexture)
{
	if (NULL == srcTexture)	return -1;
	if (m_pixfmt != srcTexture->getPixelFormat())	return -2;
	if (!isValid())	return -3;
	int texCount = 8;
	TextureResource* texArray[8] = { NULL };
	if (0 != srcTexture->getTextureResources(texArray, texCount))
	{
		return -4;
	}
	if (texCount != m_textureCount)
		return -5;
	for (int i = 0; i < texCount; i++)
	{
		TextureResource* src = texArray[i];
		TextureResource* dst = m_textureArray[i];
		if (dst == NULL || src == NULL)
			return -6;
		dst->copyResource(src);
	}
	return 0;
}

bool zRender::RawFrameTextureBase::isValid() const
{
	if (m_textureCount <= 0 || m_textureArray == NULL)	return false;
	for (size_t i = 0; i < m_textureCount; i++)
	{
		if (NULL == m_textureArray[i])
			return false;
	}
	return true;
}

int zRender::RawFrameTextureBase::acquireSync(int key, unsigned int timeout)
{
	if (!isValid())	return -3;
	for (int i = 0; i < m_textureCount; i++)
	{
		TextureResource* texRes = m_textureArray[i];
		if (texRes == NULL)
			return -1;
		texRes->acquireSync(key, timeout);
	}
	return 0;
}

int zRender::RawFrameTextureBase::releaseSync(int key)
{
	if (!isValid())	return -3;
	for (int i = 0; i < m_textureCount; i++)
	{
		TextureResource* texRes = m_textureArray[i];
		if (texRes == NULL)
			return -1;
		texRes->releaseSync(key);
	}
	return 0;
}

void RawFrameTextureBase::initTextureArray(PIXFormat pixfmt)
{
	int count = 0;
	switch (pixfmt)
	{
	case zRender::PIXFMT_YUV420P:
		count = 3;
		break;
	case zRender::PIXFMT_B8G8R8X8:
	case zRender::PIXFMT_B8G8R8A8:
	case zRender::PIXFMT_R8G8B8A8:
	case zRender::PIXFMT_R8G8B8X8:
	case zRender::PIXFMT_R8G8B8:
	case zRender::PIXFMT_A8R8G8B8:
		count = 1;
		break;
	case zRender::PIXFMT_YUY2:
	case zRender::PIXFMT_YV12:
	case zRender::PIXFMT_NV12:
		count = 2;
		break;
	case zRender::PIXFMT_UNKNOW:
	default:
		break;
	}
	if (0 >= count)	return;
	m_textureArray = (TextureResource**)malloc(sizeof(TextureResource*)*(count + 1));
	for (size_t i = 0; i < count+1; i++)
	{
		m_textureArray[i] = NULL;
	}
	if (m_textureArray)	m_textureCount = count;
}

void RawFrameTextureBase::releaseTextureArray()
{
	if (m_textureArray) free(m_textureArray);
	m_textureArray = NULL;
	m_textureCount = 0;
}