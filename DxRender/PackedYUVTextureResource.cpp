#include "PackedYUVTextureResource.h"
#include "ConstDefine.h"

using namespace zRender;

zRender::PackedYUVTextureResource::PackedYUVTextureResource()
	: m_textureRes(nullptr), m_parityTexRes(nullptr)
	, m_originPixfmt(PIXFMT_UNKNOW)
{
}

zRender::PackedYUVTextureResource::~PackedYUVTextureResource()
{
	release();
}

bool isSupportPixelFmt(PIXFormat pixfmt)
{
	switch (pixfmt)
	{
	case PIXFMT_YUY2:
		return true;
	default:
		return false;
	}
	return false;
}

int zRender::PackedYUVTextureResource::create(DxRender* render, const TextureSourceDesc& srcDesc, TEXTURE_USAGE usage)
{
	// only surport YUY2
	if (!isSupportPixelFmt(srcDesc.pixelFmt))
		return DXRENDER_RESULT_OPT_NOT_SUPPORT;
	TextureResource* texture = new TextureResource();
	int ret = texture->create((ID3D11Device*)render->getDevice(), srcDesc.width, srcDesc.height, DXGI_FORMAT_R8G8_UNORM, usage, srcDesc.isShared,
		srcDesc.buffers[0], srcDesc.pitchs[0] * srcDesc.height, srcDesc.pitchs[0]);
	if (ret != DXRENDER_RESULT_OK)
	{
		delete texture;
		return ret;
	}
	TextureResource* parityTexture = new TextureResource();
	unsigned char* parityBuf = (unsigned char*)malloc(srcDesc.width * srcDesc.height);
	for (int indexHeight = 0; indexHeight < srcDesc.height; indexHeight++)
	{
		for (int indexWidth = 0; indexWidth < srcDesc.width; indexWidth++)
		{
			parityBuf[srcDesc.width * indexHeight + indexWidth] = (indexWidth % 2) * 255;
		}
	}
	ret = parityTexture->create((ID3D11Device*)render->getDevice(), srcDesc.width, srcDesc.height, DXGI_FORMAT_R8_UNORM,
		TEXTURE_USAGE_DEFAULT, false, (char*)parityBuf, srcDesc.width * srcDesc.height, srcDesc.width);
	if (ret != DXRENDER_RESULT_OK)
	{
		delete texture;
		delete parityTexture;
		return ret;
	}
	m_textureRes = texture;
	m_parityTexRes = parityTexture;
	m_originPixfmt = srcDesc.pixelFmt;
	return ret;
}

void zRender::PackedYUVTextureResource::release()
{
	if (m_textureRes != nullptr)
	{
		m_textureRes->release();
		m_textureRes = nullptr;
	}
	if (m_parityTexRes != nullptr)
	{
		m_parityTexRes->release();
		m_parityTexRes = nullptr;
	}
}

int zRender::PackedYUVTextureResource::createResourceView()
{
	if (!valid())
		return DXRENDER_RESULT_NOT_INIT_YET;
	int ret = m_textureRes->createResourceView();
	int ret2 = m_parityTexRes->createResourceView();
	if (ret != DXRENDER_RESULT_OK || ret2 != DXRENDER_RESULT_OK)
	{
		m_textureRes->releaseResourceView();
		m_parityTexRes->releaseResourceView();
	}
	return ret;
}

void zRender::PackedYUVTextureResource::releaseResourceView()
{
	if (valid())
	{
		m_textureRes->releaseResourceView();
		m_parityTexRes->releaseResourceView();
	}
}

int zRender::PackedYUVTextureResource::update(const TextureSourceDesc& srcDesc)
{
	if (!valid())
		return DXRENDER_RESULT_NOT_INIT_YET;
	return m_textureRes->update(srcDesc);
}

int zRender::PackedYUVTextureResource::getTextures(ID3D11Texture2D** outTexs, int& texsCount) const
{
	if (nullptr == outTexs || texsCount < 2)
		return DXRENDER_RESULT_PARAM_INVALID;
	if (!valid())
		return DXRENDER_RESULT_NOT_INIT_YET;
	int count = 1;
	int ret = m_textureRes->getTextures(outTexs, count);
	if (ret != DXRENDER_RESULT_OK || count != 1)
		return DXRENDER_RESULT_INTERNAL_ERR;
	ret = m_parityTexRes->getTextures(outTexs + 1, count);
	if (ret != DXRENDER_RESULT_OK || count != 1)
		return DXRENDER_RESULT_INTERNAL_ERR;
	texsCount = 2;
	return DXRENDER_RESULT_OK;
}

int zRender::PackedYUVTextureResource::getResourceView(ID3D11ShaderResourceView** outSRVs, int& srvsCount) const
{
	if (nullptr == outSRVs || srvsCount < 2)
		return DXRENDER_RESULT_PARAM_INVALID;
	if (!valid())
		return DXRENDER_RESULT_NOT_INIT_YET;
	int count = 1;
	int ret = m_textureRes->getResourceView(outSRVs, count);
	if (ret != DXRENDER_RESULT_OK || count != 1)
		return DXRENDER_RESULT_INTERNAL_ERR;
	ret = m_parityTexRes->getResourceView(outSRVs + 1, count);
	if (ret != DXRENDER_RESULT_OK || count != 1)
		return DXRENDER_RESULT_INTERNAL_ERR;
	srvsCount = 2;
	return DXRENDER_RESULT_OK;
}

int zRender::PackedYUVTextureResource::copyResource(const ITextureResource* res)
{
	return DXRENDER_RESULT_NOT_IMPLETE;
}

ITextureResource* zRender::PackedYUVTextureResource::copy()
{
	return nullptr;
}

int zRender::PackedYUVTextureResource::acquireSync(int key, unsigned int timeout)
{
	if (!valid())
		return DXRENDER_RESULT_NOT_INIT_YET;
	return m_textureRes->acquireSync(key, timeout);
}

int zRender::PackedYUVTextureResource::releaseSync(int key)
{
	if (!valid())
		return DXRENDER_RESULT_NOT_INIT_YET;
	return m_textureRes->releaseSync(key);
}

bool zRender::PackedYUVTextureResource::valid() const
{
	return m_textureRes!=nullptr && m_parityTexRes!=nullptr;
}
