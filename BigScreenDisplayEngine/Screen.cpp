#include "Screen.h"
#include "ScreenRender.h"
#include "BigViewport.h"

using namespace SOA::Mirror::Render;

Screen::Screen(const ScreenConfig& screenCfg, BigScreenBackground* background)
{
	if (screenCfg.width <= 0 || screenCfg.height <= 0 || screenCfg.screenCellCfg.size() > screenCfg.width*screenCfg.height)
		throw std::exception("Argument invalid.");
	for (size_t cellIndex = 0; cellIndex < screenCfg.screenCellCfg.size(); cellIndex++)
	{
		ScreenRender* scRender = NULL;
		try{
			scRender = new ScreenRender(screenCfg.screenCellCfg[cellIndex].output, 
				screenCfg.screenCellCfg[cellIndex].posX, screenCfg.screenCellCfg[cellIndex].posY,
				background);
			m_screenRender.push_back(scRender);
		}
		catch (const std::exception& ex)
		{
			delete scRender;
			printf("Create Screen Render obj failed.(%s)\n", ex.what());
		}
	}
	if (m_screenRender.size() <= 0)
		throw std::exception("Create Screen Render Obj failed.(Can not create any obj)\n");
}

Screen::~Screen()
{
	for (size_t scRenderIndex = 0; scRenderIndex < m_screenRender.size(); scRenderIndex++)
	{
		delete m_screenRender[scRenderIndex];
	}
	m_screenRender.clear();
}

ScreenRender* Screen::getScreenRender(int posX, int posY) const
{
	for (size_t scRenderIndex = 0; scRenderIndex < m_screenRender.size(); scRenderIndex++)
	{
		ScreenRender* sr = m_screenRender[scRenderIndex];
		if (!sr)	continue;
		if (static_cast<int>(sr->getPosX()) == posX
			&& static_cast<int>(sr->getPosY()) == posY)
			return sr;
	}
	return NULL;
}

BigViewport* Screen::createViewport(const zRender::RECT_f& viewportReg, int zIndex)
{
	try{
		BigViewport* vp = new BigViewport(viewportReg, zIndex, *this);
		return vp;
	}
	catch (const std::exception& ex)
	{
		printf("Error in Screen::createViewport : %s\n", ex.what());
	}
	catch (...)
	{
		printf("Error in Screen::createViewport : Unknow Exception.\n");
	}
	return NULL;
}

void Screen::destroyViewport(BigViewport** bigviewport)
{
	if (bigviewport)
	{
		delete *bigviewport;
		*bigviewport = NULL;
	}
}