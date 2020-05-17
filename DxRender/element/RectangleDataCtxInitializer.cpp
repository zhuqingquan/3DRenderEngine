#include "element/RectangleDataCtxInitializer.h"
#include "element/RectMetaData.h"
#include "RectDrawingContext.h"
#include "DXLogger.h"
#include "ConstDefine.h"

using namespace zRender;

static const TCHAR* TAG = _T("RectangleDataCtxInitializer");

RectangleDataCtxInitializer::RectangleDataCtxInitializer()
	: m_metaData(new RectMetaData())
	, m_drawingCtx(nullptr)
{

}

RectangleDataCtxInitializer::~RectangleDataCtxInitializer()
{
	deinit();
}

int RectangleDataCtxInitializer::init(DxRender* render)
{
	if (render == nullptr)
		return DXRENDER_RESULT_NOT_INIT_YET;
	if (0 != m_metaData->init(render, _T("G:\\´úÂëºÚ¶´\\3DRenderEngine\\DxRender\\FX\\DefaultVideo.fxo")))
	{
		log_e(TAG, _T("Init meta data failed."));
		return DXRENDER_RESULT_SUBCOMPONENT_INIT_FAILED;
	}
	m_drawingCtx = new RectDrawingContext(render->getWidth(), render->getHeight());
	m_drawingCtx->setDsplModel(m_metaData->getDsplModel());
	return 0;
}

void RectangleDataCtxInitializer::deinit()
{
	delete m_metaData;
	m_metaData = nullptr;
	delete m_drawingCtx;
	m_drawingCtx = NULL;
}
