#pragma once
#ifndef _Z_RENDER_ZRECT_H_
#define _Z_RENDER_ZRECT_H_

#include "DisplayElement.h"
#include "DxZRenderDLLDefine.h"

namespace zRender
{
	class DX_ZRENDER_EXPORT_IMPORT ZRect : public DisplayElement
	{
	public:
		ZRect(DxRender* dxRender);
		virtual ~ZRect();

		static ZRect* create(DxRender* dxRender, const RECT_f& displayReg, int zIndex);
		static void release(ZRect** rectElem);
	protected:
		/**
		 *	@name		init
		 *	@brief		初始化，完成MetaData、DrawingContext的初始化
						构造函数中将直接调用这个方法进行环境初始化
		 *	@return		int 0--成功  其他--失败
		 **/
		virtual int init(DxRender* dxRender);

		/**
		 *	@name		deinit
		 *	@brief		释放init调用中创建的资源
		 *	@return		int 0--成功  其他--失败
		 **/
		virtual void deinit();
	};
}

#endif//_Z_RENDER_ZRECT_H_