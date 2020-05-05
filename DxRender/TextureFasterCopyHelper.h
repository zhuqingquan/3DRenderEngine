#pragma once
#ifndef _Z_RENDER_TEXTURE_FASTER_COPY_HELPER_H_
#define _Z_RENDER_TEXTURE_FASTER_COPY_HELPER_H_

#include "DxZRenderDLLDefine.h"
#include "DxRender.h"
#include "inc/TextureResource.h"

namespace zRender
{
	/**
	 *	@name	TextureFasterCopyHelper
	 *	@brief	实现优化将内存数据拷贝到Texture中的效率的操作
	 *			此协助类实现的方式时默认创建两个同样格式的Texture，一种USAGE为Stage，一种为Default
	 *			当需要将内存数据拷贝到Texture中时，先拷贝到Stage的Texture中，再由Stage拷贝到Default中
	 **/
	class DX_ZRENDER_EXPORT_IMPORT TextureFasterCopyHelper
	{
	public:
		TextureFasterCopyHelper();
		~TextureFasterCopyHelper();

		int create(DxRender* render, const TextureSourceDesc& srcDesc);
		void release();
		int update(const TextureSourceDesc& srcDesc);

		ITextureResource* getTextureForDraw() const { return m_defaultTex; }
		ITextureResource* getTextureForUpdate() const { return m_stageTex; }
	private:
		TextureFasterCopyHelper(const TextureFasterCopyHelper&);
		TextureFasterCopyHelper& operator=(const TextureFasterCopyHelper&);

		ITextureResource* m_stageTex;
		ITextureResource* m_defaultTex;
	};
}

#endif//_Z_RENDER_TEXTURE_FASTER_COPY_HELPER_H_