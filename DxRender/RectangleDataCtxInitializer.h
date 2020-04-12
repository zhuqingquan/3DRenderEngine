#pragma once
#ifndef _ZRENDER_RECTANGLE_DATA_CTX_INITIALIZER_H_
#define _ZRENDER_RECTANGLE_DATA_CTX_INITIALIZER_H_

#include "ElementDataCtxInitializer.h"

namespace zRender
{
	/**
	 *	@name	RectangleDataCtxInitializer
	 *	@brief	矩形区域的DisplayElement的资源初始化
	 **/
	class RectangleDataCtxInitializer : public ElementDataCtxInitializer
	{
	public:
		RectangleDataCtxInitializer();
		~RectangleDataCtxInitializer();

		virtual int init(DxRender* render);
		virtual void deinit();

		virtual ElementMetaData* GetMetaData() { return m_metaData; }
		virtual ElementDrawingContext* GetDrawingContext() { return m_drawingCtx; }

	private:
		ElementMetaData* m_metaData;
		ElementDrawingContext* m_drawingCtx;
	};
}

#endif//_ZRENDER_RECTANGLE_DATA_CTX_INITIALIZER_H_
