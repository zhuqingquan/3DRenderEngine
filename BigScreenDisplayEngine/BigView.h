/**
 *	@author		zhuqingquan
 *	@date		2014-10-28
 *	@name		BigView.h
 *	@brief		显示内容的提供者，BigView提供BigViewport显示时所需的顶点信息、纹理信息、Shader信息，以及其他信息
 *				该类为基类，提供接口以及默认实现，用户可以通过继承该接口提供更丰富类型的BigView
 */
#pragma once
#ifndef _SOA_MIRROR_RENDER_BIGVIEW_H_
#define _SOA_MIRROR_RENDER_BIGVIEW_H_

#include "IDisplayContentProvider.h"
#include "DxRenderCommon.h"
#include <list>

namespace SOA
{
namespace Mirror
{
namespace Render
{
	class BigViewportPartition;

	class BigView
	{
	public:
		BigView(const zRender::RECT_f& effectiveReg);
		virtual ~BigView();

		virtual zRender::IDisplayContentProvider* applyAuthorization(BigViewportPartition* bvpp);
		virtual int releaseAutorization(BigViewportPartition* bvpp);
		virtual int setEffectiveReg(const zRender::RECT_f& effectiveReg);
		virtual bool isNeedShow() const;

	private:
		int createContentProvider();
		void releaseContentProvider();

		std::list<BigViewportPartition*> m_authorizatedViewportPartion;
		zRender::RECT_f m_effectiveReg;
		zRender::IDisplayContentProvider* m_contentProvider;
	};
}//namespace Render
}//namespace Mirror
}//namespace SOA

#endif //_SOA_MIRROR_RENDER_BIGVIEW_H_