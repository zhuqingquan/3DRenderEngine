#include "ScreenRender.h"
#include "RenderDrawing.h"
#include <assert.h>

using namespace SOA::Mirror::Render;

ScreenRender::ScreenRender(IDXGIOutput* dxgiOutput, float posX, float posY, BigScreenBackground* background)
	: m_rd(NULL)
	, m_window(NULL)
	, m_timerHandle(NULL)
{
	if (posX < 0 || posY < 0)
		throw std::exception("Argument invalid.");

	DXGI_OUTPUT_DESC outputDesc;
	if (dxgiOutput==NULL || S_OK != dxgiOutput->GetDesc(&outputDesc))
	{
		char msg[512] = { 0 };
		sprintf(msg, "Error in ScreenRender::ScreenRender : get output description of output failed.(X=%f Y=%f)\n", posX, posY);
		throw std::exception(msg);
	}
	WindowModel* wm = new WindowModel();//×óÉÏ
	//RECT winRect = { 0, 0, 683, 384 };
	wm->setWindowRect(outputDesc.DesktopCoordinates);
	HWND hWnd = wm->createWindows();
	wm->showWindow();
	assert(hWnd != INVALID_HANDLE_VALUE);


	HANDLE eventHandle = CreateEvent(NULL, false, false, NULL);
	assert(eventHandle != INVALID_HANDLE_VALUE);
	SOA::Mirror::Render::RenderDrawing* rd = new SOA::Mirror::Render::RenderDrawing(hWnd, posX, posY, posX + 1.0f, posY + 1.0f, background);
	if (0 != rd->start(eventHandle))
	{
		//printf("Error in  ScreenRender::ScreenRender : start render drawing failed.\n");
		delete wm;
		CloseHandle(eventHandle);
		delete rd;
		throw std::exception("Error in  ScreenRender::ScreenRender : start render drawing failed.");
	}

	m_rd = rd;
	m_window = wm;
	m_timerHandle = eventHandle;
	m_DisplayReg.left = posX;
	m_DisplayReg.right = posX + 1.0f;
	m_DisplayReg.top = posY;
	m_DisplayReg.bottom = posY + 1.0f;
}

ScreenRender::~ScreenRender()
{
	if (m_rd)
	{
		m_rd->stop();
		delete m_rd;
		m_rd = NULL;
	}
	if (m_window)
	{
		delete m_window;
		m_window = NULL;
	}
	CloseHandle(m_timerHandle);
}

RenderDrawing* ScreenRender::getRenderDrawing() const
{
	return m_rd;
}

HANDLE ScreenRender::getTimerEventHandle() const
{
	return m_timerHandle;
}