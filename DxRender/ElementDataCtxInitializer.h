#pragma once
#ifndef _ZRENDER_ELEMENT_DATA_CTX_INITIALIZER_H_
#define _ZRENDER_ELEMENT_DATA_CTX_INITIALIZER_H_

#include "d3d11.h"

namespace zRender
{
	class ElementMetaData;
	class ElementDrawingContext;
	class DxRender;

	/**
	 *	@name	ElementDataCtxInitializer
	 *	@brief	接口类，初始化DisplayElement显示所需要的MetaData与DrawingContext
	 **/
	class ElementDataCtxInitializer
	{
	public:
		virtual ~ElementDataCtxInitializer() = 0 {};

		virtual int init(DxRender* render) = 0;
		virtual void deinit() = 0;

		virtual ElementMetaData* GetMetaData() = 0;
		virtual ElementDrawingContext* GetDrawingContext() = 0;
	};
}

#endif//_ZRENDER_ELEMENT_DATA_CTX_INITIALIZER_H_
