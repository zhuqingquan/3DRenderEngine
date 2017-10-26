#pragma once
#ifndef _Z_RENDER_TEXTURE_RESOURCE_H_
#define _Z_RENDER_TEXTURE_RESOURCE_H_

#include "DxRenderCommon.h"
#include "DxZRenderDLLDefine.h"
#include <D3D11.h>

namespace zRender
{
	class TextureResource
	{
	private:
		ID3D11Texture2D*			m_rgbTex;
		ID3D11ShaderResourceView*	m_rgbSRV;
		int m_width;
		int m_height;
		PIXFormat m_pixfmt;
	public:
		TextureResource()
			: m_width(0), m_height(0)
			, m_rgbTex(NULL)
			, m_rgbSRV(NULL)
		{

		}

		int create(ID3D11Device* device, int width, int height, TEXTURE_USAGE usage, bool bShared, const char* initData, int dataLen, int pitch);
		void destroy();

		int update_A8R8G8B8(const unsigned char* pData, int dataLen, int dataPitch, int width, int height,
			const RECT& regionUpdated, ID3D11DeviceContext* d3dDevContex);
		int update_R8G8B8(const unsigned char* pData, int dataLen, int dataPitch, int width, int height,
			const RECT& regionUpdated, ID3D11DeviceContext* d3dDevContex);
		int getTexture(ID3D11Texture2D** outYUVTexs, int& texsCount) const;
		int getShaderResourceView(ID3D11ShaderResourceView** outYUVSRVs, int& srvsCount) const;
		bool valid() const;
	};
}

#endif //_Z_RENDER_TEXTURE_RESOURCE_H_