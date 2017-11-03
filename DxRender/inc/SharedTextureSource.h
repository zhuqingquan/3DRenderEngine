#pragma once
#ifndef _Z_RENDER_SHARED_TEXTURE_SOURCE_H_
#define _Z_RENDER_SHARED_TEXTURE_SOURCE_H_

#include "IDisplayContentProvider.h"
#include "DxZRenderDLLDefine.h"

namespace zRender
{
	class DxRender;

	/**
	 *	@name		SharedTextureSource
	 *	@brief		实现纹理数据获取、更新的接口
	 *				这种类型的Texture源包含2个IRawFrameTexture对象，分别为Stage类型和Shared
	 *				调用copyDataToTexture将内存数据拷贝到Stage类型的Texture中，再copy到Shared类型的Texture中
	 *				调用getTexture将获取到Shared类型的Texture，Stage类型的Texture外部不可见
	 *				getData方法不可用
	 **/
	class DX_ZRENDER_EXPORT_IMPORT SharedTextureSource : public TextureDataSource
	{
	public:
		SharedTextureSource(DxRender* render);
		~SharedTextureSource();
		virtual bool isUpdated(int identify) const;
		virtual int getTextureProfile(const RECT_f& textureReg, int& dataLen, int& yPitch, int& uPitch, int& vPitch, int& width, int& height, PIXFormat& pixelFmt);
		virtual IRawFrameTexture* getTexture();
		virtual int copyDataToTexture(const RECT_f& textureReg, unsigned char* dstTextureData, int pitch, int height, int& identify);

		int createTexture(PIXFormat pixfmt, int w, int h);
		void releaseTexture();

		void cacheData(const RECT_f& textureReg, unsigned char* dstTextureData, int pitch, int width, int height);
	private:
		virtual SharedTexture* getSharedTexture(RECT& effectReg, int& identify);
		virtual unsigned char* getData(int& dataLen, int& yPitch, int& uPitch, int& vPitch, int& width, int& height, PIXFormat& pixelFmt, RECT& effectReg, int& identify);
	private:
		IRawFrameTexture* m_texStaging;
		IRawFrameTexture* m_texShared;
		DxRender* m_dxrender;

		int m_isUpdatedIdentify;

		unsigned char* m_cacheData;
		int m_cache_pitch;
		int m_cache_width;
		int m_cache_height;
		RECT_f m_cache_textureReg;
	};
}

#endif // !_Z_RENDER_SHARED_TEXTURE_SOURCE_H_
