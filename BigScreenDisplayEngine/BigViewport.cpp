#include "BigViewport.h"
#include "ScreenRender.h"
#include "Screen.h"
#include "BigViewportPartition.h"
#include <assert.h>
#include "RenderDrawing.h"

using namespace SOA::Mirror::Render;
using namespace zRender;

BigViewport::BigViewport(const zRender::RECT_f& regOfScreen, int zIndex, const Screen& parentSc)
	: m_parentSc(parentSc)
{
	if (regOfScreen.width() <= 0 || regOfScreen.height() <= 0 || zIndex < 0)
		throw std::exception("Invalid Argument.");
	int xPosStart = static_cast<int>(regOfScreen.left);
	int xPosEnd = regOfScreen.right - static_cast<int>(regOfScreen.right) > 0.001 ? static_cast<int>(regOfScreen.right) : static_cast<int>(regOfScreen.right) - 1;
	int yPosStart = static_cast<int>(regOfScreen.top);
	int yPosEnd = regOfScreen.bottom - static_cast<int>(regOfScreen.bottom) > 0.001 ? static_cast<int>(regOfScreen.bottom) : static_cast<int>(regOfScreen.bottom) - 1;
	for (int yIndex = yPosStart; yIndex <= yPosEnd; yIndex++)
	{
		float top = yIndex == yPosStart ? regOfScreen.top : yIndex;
		float bottom = yIndex == yPosEnd ? regOfScreen.bottom : yIndex + 1;
		for (int xIndex = xPosStart; xIndex <= xPosEnd; xIndex++)
		{
			ScreenRender* scRender = parentSc.getScreenRender(xIndex, yIndex);
			RenderDrawing* rd = NULL;
			if (!scRender || (rd=scRender->getRenderDrawing())==NULL)
			{
				printf("Have Not create ScreenRender obj for (%d, %d) Cell.\n", xIndex, yIndex);
				continue;
			}
			float left = xIndex == xPosStart ? regOfScreen.left : xIndex;
			float right = xIndex == xPosEnd ? regOfScreen.right : xIndex + 1;
			RECT_f vppRegOfSc(left, right, top, bottom);
			float leftOfvp = (left - regOfScreen.left) / regOfScreen.width();
			float rightOfvp = (right - regOfScreen.left) / regOfScreen.width();
			float topOfvp = (top - regOfScreen.top) / regOfScreen.height();
			float bottomOfvp = (bottom - regOfScreen.top) / regOfScreen.height();
			RECT_f vppRegOfVp(leftOfvp, rightOfvp, topOfvp, bottomOfvp);
			BigViewportPartition* vpp = new BigViewportPartition(vppRegOfSc, vppRegOfVp, rd);
			assert(vpp);
			rd->addBigViewportPartition(vpp);
			vpp->setZIndex(zIndex);
			m_partitions.push_back(vpp);
		}
	}
}

BigViewport::~BigViewport()
{
	for (size_t i = 0; i < m_partitions.size(); i++)
	{
		delete m_partitions[i];
	}
	m_partitions.clear();
}

bool BigViewport::attachView(BigView* bigview)
{
	for (size_t i = 0; i < m_partitions.size(); i++)
	{
		m_partitions[i]->attachBigView(bigview);
	}
	return true;
}