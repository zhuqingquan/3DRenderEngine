#pragma once
#ifndef _SOA_MIRROR_RENDER_SCREENRENDER_H_
#define _SOA_MIRROR_RENDER_SCREENRENDER_H_

#include "WindowModel.h"
#include <DXGI.h>
#include "DxRenderCommon.h"
#include <exception>

namespace SOA
{
namespace Mirror
{
namespace Render
{
	class RenderDrawing;
	class BigScreenBackground;

	class ScreenRender
	{
	private:
		RenderDrawing* m_rd;
		WindowModel* m_window;
		HANDLE m_timerHandle;
		zRender::RECT_f m_DisplayReg;

	public:
		ScreenRender(IDXGIOutput* dxgiOutput, float posX, float posY, BigScreenBackground* background) throw (std::exception);
		~ScreenRender();

		RenderDrawing* getRenderDrawing() const;
		HANDLE getTimerEventHandle() const;

		float getPosX() const {	return m_DisplayReg.left; }
		float getPosY() const { return m_DisplayReg.top; }

	private:
		ScreenRender(const ScreenRender&);
		ScreenRender& operator=(const ScreenRender&);
	};
}
}
}

#endif //_SOA_MIRROR_RENDER_SCREENRENDER_H_