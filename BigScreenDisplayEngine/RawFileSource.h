#pragma once
#ifndef _Z_RENDER_RAW_FILE_SOURCE_H_
#define _Z_RENDER_RAW_FILE_SOURCE_H_

#include "inc/SharedTextureSource.h"
#include <fstream>

namespace SOA
{
namespace Mirror
{
namespace Render
{
	class BigView;
}
}
}

namespace zRender
{
	class DxRender;

	class RawFileSource
	{
	public:
		RawFileSource(DxRender* dxrender);
		~RawFileSource();

		bool open(const TCHAR* filePathName, zRender::PIXFormat pixfmt, int width, int height);
		void close();

		bool start(int fps);
		void stop();
		void doReadFileAndUpdateToTexture();

		SOA::Mirror::Render::BigView* createSourceView();
		void releaseSourceView(SOA::Mirror::Render::BigView** srcView);

		zRender::SharedTextureSource* getTextureSource() const { return m_textureSource; }
	private:
		zRender::SharedTextureSource* m_textureSource;

		std::ifstream* m_fileStream;
		int m_width;
		int m_height;
		zRender::PIXFormat m_pixfmt;
		HANDLE m_thUpdate;
		bool m_thUpdateRunning;
		int m_fps;
	};
}

#endif //_Z_RENDER_RAW_FILE_SOURCE_H_
