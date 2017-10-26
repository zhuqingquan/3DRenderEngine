#include "inc/TextureResource.h"

using namespace zRender;

TextureResource::TextureResource()
	: m_width(0), m_height(0)
	, m_dxgifmt(DXGI_FORMAT_UNKNOWN), m_usage(TEXTURE_USAGE_DEFAULT)
	, m_texture(NULL)
	, m_rsv(NULL)
	, m_device(NULL), m_contex(NULL)
	, m_bShared(false), m_sharedHandle(INVALID_HANDLE_VALUE)
{

}

TextureResource::~TextureResource()
{
	releaseResourceView();
	release();
}

int TextureResource::create(ID3D11Device* device, int width, int height, DXGI_FORMAT dxgifmt, TEXTURE_USAGE usage, bool bShared, const char* initData, int dataLen, int pitch)
{
	return -1;
}

int TextureResource::open(HANDLE sharedHandle)
{
	return -1;
}

void TextureResource::release()
{

}

int TextureResource::createResourceView()
{
	return -1;
}

void TextureResource::releaseResourceView()
{

}

int TextureResource::copyResource(const TextureResource* res)
{
	return -1;
}

int TextureResource::update(const unsigned char* pData, int dataLen, int dataPitch, int width, int height,
	const RECT& regionUpdated)
{
	return -1;
}