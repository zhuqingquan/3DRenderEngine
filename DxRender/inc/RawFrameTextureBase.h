#pragma once
#ifndef _Z_RENDER_RAW_FRAME_TEXTURE_BASE_H_
#define _Z_RENDER_RAW_FRAME_TEXTURE_BASE_H_

#include "IRawFrameTexture.h"

namespace zRender
{
	class RawFrameTextureBase : public IRawFrameTexture
	{
	public:
		RawFrameTextureBase(PIXFormat pixfmt);
		~RawFrameTextureBase();

		//virtual int create(ID3D11Device* device, int width, int height, TEXTURE_USAGE usage, bool bShared, const char* initData, int dataLen, int pitch);
		virtual int openSharedTexture(ID3D11Device* device, IRawFrameTexture* sharedTexture);
		virtual int getTextureResources(TextureResource** outTexs, int& texsCount) const;

		virtual int copyTexture(const IRawFrameTexture* srcTexture);

		bool isValid() const;

		virtual int acquireSync(int key, unsigned int timeout);
		virtual int releaseSync(int key);
	protected:
		TextureResource** m_textureArray;
		int m_textureCount;

	private:
		void initTextureArray(PIXFormat pixfmt);
		void releaseTextureArray();
	};
}

#endif // !_Z_RENDER_RAW_FRAME_TEXTURE_BASE_H_

