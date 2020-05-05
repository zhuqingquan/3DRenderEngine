#include "TextureFasterCopyHelper.h"
#include "ConstDefine.h"
#include <TextureResourceFactory.h>

using namespace zRender;

zRender::TextureFasterCopyHelper::TextureFasterCopyHelper()
	: m_defaultTex(nullptr)
	, m_stageTex(nullptr)
{
}

zRender::TextureFasterCopyHelper::~TextureFasterCopyHelper()
{
	release();
}

int zRender::TextureFasterCopyHelper::create(DxRender* render, const TextureSourceDesc& srcDesc)
{
	if (m_defaultTex != nullptr || m_stageTex != nullptr)
		return DXRENDER_RESULT_FUNC_REENTRY_INVALID;
	m_defaultTex = TextureResourceFactory::create(srcDesc.pixelFmt);
	m_stageTex = TextureResourceFactory::create(srcDesc.pixelFmt);
	if (m_defaultTex == nullptr || m_stageTex == nullptr)
	{
		return DXRENDER_RESULT_OPT_NOT_SUPPORT;
	}
	int ret = m_defaultTex->create(render, srcDesc, TEXTURE_USAGE_DEFAULT);
	if (ret != DXRENDER_RESULT_OK)
		return ret;
	ret = m_stageTex->create(render, srcDesc, TEXTURE_USAGE_STAGE);

	return ret;
}

void zRender::TextureFasterCopyHelper::release()
{
	if (m_defaultTex != nullptr)
	{
		m_defaultTex->release();
		m_defaultTex = nullptr;
	}

	if (m_stageTex!= nullptr)
	{
		m_stageTex->release();
		m_stageTex = nullptr;
	}
}

int zRender::TextureFasterCopyHelper::update(const TextureSourceDesc& srcDesc)
{
	if (m_stageTex == nullptr || m_defaultTex == nullptr)
		return DXRENDER_RESULT_NOT_INIT_YET;
	int ret = m_stageTex->update(srcDesc);
	if (ret != DXRENDER_RESULT_OK)
		return ret;
	ret = m_defaultTex->copyResource(m_stageTex);
	
	return ret;
}
