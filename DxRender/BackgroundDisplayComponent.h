/**
 *	@author		zhuqingquan
 *	@date		2014-10-17
 *	@name		BackgroundDisplayComponent.h
 *	@brief		Interface of the display engine
 */

#pragma once
#ifndef _ZRENDER_BACKGROUND_DISPLAY_COMPONENT_H_
#define _ZRENDER_BACKGROUND_DISPLAY_COMPONENT_H_

//#include "DxZRenderDLLDefine.h"

namespace zRender
{
	class DisplayElement;
	class IDisplayContentProvider;

	/**
	 *	@name		BackgroundComponent
	 *	@brief		引擎背景内容的模块，保存显示背景内容所需要的相关信息
	 **/
	class BackgroundComponent
	{
	public:
		/**
		 *	@name		BackgroundComponent
		 *	@brief		构造函数，以显示所需的相关信息为参数构造，显示引擎将使用这些信息进行渲染
		 *	@param[in]	IDisplayContentProvider* contentProvider 背景内容的提供者
		 *	@param[in]	DisplayElement* de 渲染过程所需要的信息
		 **/
		BackgroundComponent(IDisplayContentProvider* contentProvider, DisplayElement* de)
		: m_ctPro(contentProvider), m_de(de)
		{}

		/**
		 *	@name	~BackgroundComponent
		 *	@brief	析构函数
		 **/
		~BackgroundComponent() { m_ctPro=0; m_de=0; }

		/**
		 *	@name		getContentProvider
		 *	@brief		获取背景内容的提供者
		 *	@return		IDisplayContentProvider* 背景内容的提供者
		 **/
		IDisplayContentProvider* getContentProvider() const { return m_ctPro; }

		/**
		 *	@name		getDisplayElement
		 *	@brief		获取渲染过程所需要的信息
		 *	@return		DisplayElement* 渲染过程所需要的信息
		 **/
		DisplayElement* getDisplayElement() const { return m_de; }

	private:
		IDisplayContentProvider* m_ctPro;
		DisplayElement* m_de;
	};
}

#endif //_zRENDER_BACKGROUND_DISPLAY_COMPONENT_H_