#pragma once
#ifndef _ZRENDER_TEXTURE_RESOURCE_FACTORY_H_
#define _ZRENDER_TEXTURE_RESOURCE_FACTORY_H_

#include "DxRenderCommon.h"

namespace zRender
{
	struct ITextureResource;

	class TextureResourceFactory
	{
	public:
		static ITextureResource* create(PIXFormat pixfmt);
	};
}

#endif//_DXRENDER_TEXTURE_RESOURCE_FACTORY_H_