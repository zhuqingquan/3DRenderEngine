#pragma once
#ifndef _ZRENDER_ELEMENT_DRAWING_CONTEXT_H_
#define _ZRENDER_ELEMENT_DRAWING_CONTEXT_H_

#include "ElemDsplModel.h"
#include "Effects.h"
#include "LightHelper.h"

namespace zRender
{
	class BasicEffect;
	class DxRender;
	class ElementMetaData;
	class ITextureResource;

	class ElementDrawingContext
	{
	public:
		ElementDrawingContext(int width, int height);


		const XMFLOAT4X4& getWorldTransformMatrix() const { return m_WorldTransformMat; }
		void setWorldTransformMatrix(const XMFLOAT4X4& worldMat);

// 		const XMFLOAT4X4& getViewTransformMatrix() const { return m_viewTransform; }
// 		void setViewTransformMatrix(const XMFLOAT4X4& viewMat);

//		const XMFLOAT4X4& getProjectionTransformMatrix() const { return m_projTransform; }
//		void setProjectionTransformMatrix(const XMFLOAT4X4& projectionMat);

		const Material& getMaterial() const;
		void setMaterial(const Material& material);

		int apply(DxRender* render, IRawFrameTexture* texture, ID3D11Buffer* vtBuf, ID3D11Buffer* indexBuffer, ElementMetaData* metadata);

		int apply(DxRender* render, ITextureResource* textures[], int textureCount,
			ID3D11Buffer* vtBuf, ID3D11Buffer* indexBuffer, ElementMetaData* metadata);


		/**
		 *	@name		setDisplayRegion
		 *	@brief		设置该显示内容在显示引擎坐标系中的位置
		 *				用户可通过重新设置该位置达到移动显示内容的作用
		 *	@param[in]	const RECT_f& displayReg	表示显示区域的的坐标系位置
		 *	@param[in]	float zIndex 当前显示原始所在顶点的Z坐标偏移
		 *	@return		int 0--成功 <0--失败	该参数不合法时失败
		 **/
		int setDisplayRegion(const RECT_f& displayReg, float zIndex);//处理移动，设置内容的显示位置

		/**
		 *	@name		setZIndex
		 *	@brief		设置该显示内容在显示引擎坐标系中的位置的Z坐标
		 *				用户可通过重新设置该位置达到移动显示内容Z坐标的作用
		 *	@param[in]	float zIndex 当前显示原始所在顶点的Z坐标偏移
		 *	@return		int 0--成功 <0--失败	该参数不合法时失败
		**/
		int setZIndex(float zIndex);

		float getZIndex() const;

		/**
		 *	@name		setDsplModel
		 *	@brief		设置使用的Display Model
		 *	@param[in]	ElemDsplModel<BasicEffect> * dsplModel ElemDsplModel对象
		 *	@return		void
		 **/
		void setDsplModel(ElemDsplModel<BasicEffect>* dsplModel) { m_dsplModel = dsplModel; }

		void enableTransparent(bool enable) { m_isEnableTransparent = enable; }
		bool isEnableTransparent() const { return m_isEnableTransparent; }

		/**
		 *	@name			setAlpha
		 *	@brief			设置显示内容透明度，有效范围（1.0f -- 0.0f）
		 *	@param[in]		float alpha 透明度，1.0f为不透明， 0.0f为全透明
		 **/
		void setAlpha(float alpha);
		/**
		 *	@name			getAlpha
		 *	@brief			获取当前设置的透明度
		 *	@return			float
		 **/
		float getAlpha() const;
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
#endif//_ZRENDER_ELEMENT_DRAWING_CONTEXT_H_
