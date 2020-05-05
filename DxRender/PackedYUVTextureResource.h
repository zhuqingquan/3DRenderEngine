#pragma once
#ifndef _Z_RENDER_PACKED_YUV_TEXTURE_RESOURCE_H_
#define _Z_RENDER_PACKED_YUV_TEXTURE_RESOURCE_H_

#include "inc/TextureResource.h"

namespace zRender
{
	class TextureResource;

	class PackedYUVTextureResource : public ITextureResource
	{
	public:
		PackedYUVTextureResource();
		virtual ~PackedYUVTextureResource();

		virtual int create(DxRender* render, const TextureSourceDesc& srcDesc, TEXTURE_USAGE usage);
		virtual void release();
		virtual int createResourceView();
		virtual void releaseResourceView();
		virtual int update(const TextureSourceDesc& srcDesc);
		virtual int getTextures(ID3D11Texture2D** outTexs, int& texsCount) const;
		virtual int getResourceView(ID3D11ShaderResourceView** outSRVs, int& srvsCount) const;
		virtual int copyResource(const ITextureResource* res);
		virtual ITextureResource* copy();
		virtual int acquireSync(int key, unsigned int timeout);
		virtual int releaseSync(int key);

		virtual int width() const { return m_textureRes!=nullptr ? m_textureRes->width() : 0; }
		virtual int height() const { return m_textureRes != nullptr ? m_textureRes->height() : 0; }
		virtual PIXFormat getSrcPixelFormat() const { return m_originPixfmt; }
		virtual bool valid() const;

	private:
		TextureResource* m_textureRes;
		TextureResource* m_parityTexRes;
		PIXFormat m_originPixfmt;
	};
}

#endif//_Z_RENDER_PACKED_YUV_TEXTURE_RESOURCE_H_
