#include "element/zRect.h"
#include "element/RectangleDataCtxInitializer.h"
#include "DXLogger.h"
#include "ConstDefine.h"

using namespace zRender;

#define LOG_TAG L"Element_ZRect"

zRender::ZRect::ZRect(DxRender* dxRender)
	: DisplayElement(dxRender)
{
	init(dxRender);
}

zRender::ZRect::~ZRect()
{
	deinit();
}

ZRect* zRender::ZRect::create(DxRender* dxRender, const RECT_f& displayReg, int zIndex)
{
	ZRect* de = new ZRect(dxRender);
	int ret = de->move(displayReg, (float)zIndex);
	if (0 != ret)
	{
		log_e(LOG_TAG, _T("Create ZRect failed."));
		return nullptr;
	}
	return de;
}

void zRender::ZRect::release(ZRect** rectElem)
{
	if (rectElem != nullptr)
	{
		delete* rectElem;
		*rectElem = nullptr;
	}
}

int zRender::ZRect::init(DxRender* dxRender)
{
	if (m_DataCtxInitializer != nullptr)
		return DXRENDER_RESULT_OK;
	m_DataCtxInitializer = new RectangleDataCtxInitializer();
	int result = m_DataCtxInitializer->init(dxRender);
	if (0 != result)
	{
		log_e(LOG_TAG, _T("Init Data context failed."));
		delete m_DataCtxInitializer;
		m_DataCtxInitializer = nullptr;
		return result;
	}
	else
	{
		m_DrawingContext = m_DataCtxInitializer->GetDrawingContext();
		m_MetaData = m_DataCtxInitializer->GetMetaData();

		m_device = (ID3D11Device*)dxRender->getDevice();
		m_context = (ID3D11DeviceContext*)dxRender->getContext();
		m_isVertexInfoUpdated = true;
	}
	return DXRENDER_RESULT_OK;
}

void zRender::ZRect::deinit()
{
	if (m_DataCtxInitializer != nullptr)
	{
		m_DataCtxInitializer->deinit();
		delete m_DataCtxInitializer;
		m_DataCtxInitializer = nullptr;
	}
}
