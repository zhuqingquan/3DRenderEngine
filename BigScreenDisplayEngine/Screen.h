#pragma once
#ifndef _SOA_MIRROR_RENDER_SCREEN_H_
#define _SOA_MIRROR_RENDER_SCREEN_H_

#include <DXGI.h>
#include <vector>
#include "DxRenderCommon.h"
#include <exception>

namespace SOA
{
namespace Mirror
{
namespace Render
{
	struct ScreenCellConfig
	{
		IDXGIOutput* output;
		float posX;
		float posY;
	};

	struct ScreenConfig
	{
	public:
		int width;
		int height;
		std::vector<ScreenCellConfig> screenCellCfg;
	};

	class BigViewport;
	class ScreenRender;
	class BigScreenBackground;

	class Screen
	{
	public:
		Screen(const ScreenConfig& screenCfg, BigScreenBackground* background)  throw (std::exception);
		~Screen();

		BigViewport* createViewport(const zRender::RECT_f& viewportReg, int zIndex);
		void destroyViewport(BigViewport** bigviewport);

		inline std::vector<ScreenRender*> getScreenRender() const;
		ScreenRender* getScreenRender(int posX, int posY) const;
	private:
		std::vector<ScreenRender*> m_screenRender;

	private:
		Screen(const Screen& sc);
		Screen& operator=(const Screen& sc);
	};

	std::vector<ScreenRender*> Screen::getScreenRender() const
	{
		return m_screenRender;
	}
}
}
}

#endif //_SOA_MIRROR_RENDER_SCREEN_H_