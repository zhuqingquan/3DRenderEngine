#pragma once
#ifndef _Z_RENDER_SHARED_TEXTURE_SOURCE_H_
#define _Z_RENDER_SHARED_TEXTURE_SOURCE_H_

#include "IDisplayContentProvider.h"
#include "DxZRenderDLLDefine.h"
#include <ConstDefine.h>
#include <inc/TextureResource.h>

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

		/**
		 *	@name		getTextureCount
		 *	@brief		获取Element所需的TextureResource的个数
		 *	@return		zRender::VertexVector*
		 **/
		virtual int getTextureCount() const { return 1; }

		/**
		 *	@name		getTextureSourceDesc
		 *	@brief		获取下标为index的Texture对应的数据源的数据格式
		 *	@param[in]	int index TextureResource的下标
		 *	@param[out] TextureSourceDesc* srcDesc 保存数据源的数据格式
		 **/
		virtual int getTextureSourceDesc(int index, TextureSourceDesc* srcDesc) const
		{
			if (index > 0 || index<0 || srcDesc == nullptr)
				return DXRENDER_RESULT_PARAM_INVALID;
			srcDesc->set(m_srcDesc);
			return DXRENDER_RESULT_OK;
		}

		/**
		 *	@name		isTextureUpdated
		 *	@brief		获取下标为index的Texture对应的数据源的数据是否发生了改变
		 *	@param[in]	int index TextureResource的下标
		 *	@param[in,out] 外部持有的最新更新的TextureResource的计数版本，如果Texture 
		 **/
		virtual bool isTextureUpdated(int index, unsigned int identify);

		/**
		 *	@name		updateTextures
		 *	@brief		将TextureDataSource中的数据更新到TextureResource中
		 *	@param[in]	TextureResource* textureArray TextureResource对象
		 *	@param[in]  int index 需要更新的Texture数据的下标Index
		 *	@param[out] unsigned int& newIdentify 成功更新后保存新的数据版本标识值，如果未更新则不更改
		 **/
		virtual int updateTextures(ITextureResource* textureArray, int index, unsigned int& newIdentify)
		{
			if (index < 0 || index >= getTextureCount() || textureArray == nullptr)
				return DXRENDER_RESULT_PARAM_INVALID;
			newIdentify = m_isUpdatedIdentify;
			return textureArray->update(m_srcDesc);
		}
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

		TextureSourceDesc m_srcDesc;
	};
}

#endif // !_Z_RENDER_SHARED_TEXTURE_SOURCE_H_
