#pragma once
#ifndef _Z_RENDER_I_TEXTURE_RESOURCE_H_
#define _Z_RENDER_I_TEXTURE_RESOURCE_H_

#include "DxRenderCommon.h"
#include "DxZRenderDLLDefine.h"
#include <D3D11.h>
#include "DxRender.h"

namespace zRender
{
	/**
	 *	@name	ITextureResource
	 *	@brief	显示中所用的Texture资源的接口定义
	 **/
	struct ITextureResource
	{
		virtual ~ITextureResource() = 0 {}

		virtual int create(DxRender* render, const TextureSourceDesc& srcDesc, TEXTURE_USAGE usage) = 0;
		virtual void release() = 0;
		virtual int createResourceView() = 0;
		virtual void releaseResourceView() = 0;
		virtual int update(const TextureSourceDesc& srcDesc) = 0;
		virtual int getTextures(ID3D11Texture2D** outTexs, int& texsCount) const = 0;
		virtual int getResourceView(ID3D11ShaderResourceView** outSRVs, int& srvsCount) const = 0;
		virtual int copyResource(const ITextureResource* res) = 0;
		virtual ITextureResource* copy() = 0;
		virtual int acquireSync(int key, unsigned int timeout) = 0;
		virtual int releaseSync(int key) = 0;

		virtual int width() const = 0;
		virtual int height() const = 0;
		virtual PIXFormat getSrcPixelFormat() const = 0;
		virtual bool valid() const = 0;
	};
}

#endif//_Z_RENDER_I_TEXTURE_RESOURCE_H_