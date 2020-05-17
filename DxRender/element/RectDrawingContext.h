#pragma once
#ifndef _Z_RENDER_RECT_DRAWING_CONTEXT_H_
#define _Z_RENDER_RECT_DRAWING_CONTEXT_H_

#include "ElementDrawingContext.h"

namespace zRender
{
	/**
	 * @name	RectDrawingContext
	 * @brief	The drawing context use to draw Rect
	 **/
	class RectDrawingContext : public ElementDrawingContext
	{
	public:
		RectDrawingContext(int width, int height);

		virtual int apply(DxRender* render, ITextureResource* textures[], int textureCount,
			ID3D11Buffer* vtBuf, ID3D11Buffer* indexBuffer, ElementMetaData* metadata);


		/**
		 *	@name		setDisplayRegion
		 *	@brief		设置该显示内容在显示引擎坐标系中的位置
		 *				用户可通过重新设置该位置达到移动显示内容的作用
		 *	@param[in]	const RECT_f& displayReg	表示显示区域的的坐标系位置
		 *	@param[in]	float zIndex 当前显示原始所在顶点的Z坐标偏移
		 *	@return		int 0--成功 <0--失败	该参数不合法时失败
		 **/
		virtual int setDisplayRegion(const RECT_f& displayReg, float zIndex);//处理移动，设置内容的显示位置

		/**
		 *	@name		setZIndex
		 *	@brief		设置该显示内容在显示引擎坐标系中的位置的Z坐标
		 *				用户可通过重新设置该位置达到移动显示内容Z坐标的作用
		 *	@param[in]	float zIndex 当前显示原始所在顶点的Z坐标偏移
		 *	@return		int 0--成功 <0--失败	该参数不合法时失败
		**/
		virtual int setZIndex(float zIndex);

		virtual float getZIndex() const;

		/**
		 *	@name		setDsplModel
		 *	@brief		设置使用的Display Model
		 *	@param[in]	ElemDsplModel<BasicEffect> * dsplModel ElemDsplModel对象
		 *	@return		void
		 **/
		virtual void setDsplModel(ElemDsplModel<BasicEffect>* dsplModel) { m_dsplModel = dsplModel; }

		virtual void enableTransparent(bool enable) { m_isEnableTransparent = enable; }
		virtual bool isEnableTransparent() const { return m_isEnableTransparent; }

		/**
		 *	@name			setAlpha
		 *	@brief			设置显示内容透明度，有效范围（1.0f -- 0.0f）
		 *	@param[in]		float alpha 透明度，1.0f为不透明， 0.0f为全透明
		 **/
		virtual void setAlpha(float alpha);
		/**
		 *	@name			getAlpha
		 *	@brief			获取当前设置的透明度
		 *	@return			float
		 **/
		virtual float getAlpha() const;
	private:

		XMFLOAT4X4			m_WorldTransformMat;
		XMFLOAT4X4			m_worldBaseTransform;
		XMFLOAT4X4			m_viewTransform;
		XMFLOAT4X4			m_projTransform;
		ElemDsplModel<BasicEffect>* m_dsplModel;
		float m_aspectRatio;
		float				m_alpha;
		bool				m_isEnableTransparent;
	};
}

#endif //_Z_RENDER_RECT_DRAWING_CONTEXT_H_