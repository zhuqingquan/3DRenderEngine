#pragma once
#ifndef _ZRENDER_ELEMENT_META_DATA_H_
#define _ZRENDER_ELEMENT_META_DATA_H_

#include "d3d11.h"
#include "DxRenderCommon.h"
#include "ElemDsplModel.h"

namespace zRender
{
	class TextureDataSource;
	class VertexVector;
	class DxRender;
	class BasicEffect;

	class ElementMetaData
	{
	public:
		virtual ~ElementMetaData() = 0 {}

		/**
		 *	@name		init
		 *	@brief		初始化Element的MetaData
		 *	@param[in]	DxRender * render 创建资源的DxRender
		 *	@param[in]	const TCHAR * shaderFileName Shader文件的路径
		 *	@return		int 0--成功  其他--失败  参考ConstDefine.h中的枚举值
		 **/
		virtual int init(DxRender* render, const TCHAR* shaderFileName) = 0;
		/**
		 *	@name		deinit
		 *	@brief		释放init中的资源
		 *	@return		void
		 **/
		virtual void deinit() = 0;

		/**
		 *	@name		isValid
		 *	@brief		MetaData是否有效
		 *	@return		bool true--有效  false--无效
		 **/
		virtual bool isValid() const = 0;

		/**
		 *	@name		getVertexData
		 *	@brief		获取顶点信息、IndexBuffer信息
		 *	@return		zRender::VertexVector*
		 **/
		virtual VertexVector* getVertexData() = 0;

		/**
		 *	@name		getTextureDataSource
		 *	@brief		获取纹理数据对象
		 *	@return		zRender::TextureDataSource*
		 **/
		virtual TextureDataSource* getTextureDataSource() const = 0;

		/**
		 *	@name		setTextureDataSource
		 *	@brief		设置纹理数据源对象
		 *	@param[in]	zRender::TextureDataSource* 纹理数据源对象
		 **/
		virtual void setTextureDataSource(TextureDataSource* textureDataSrc) = 0;
		//{
		//	m_textureDataSource = textureDataSrc;
		//}

		virtual bool isTextureNeedUpdate(int& identify) = 0;

		/**
		 *	@name		update
		 *	@brief		将数据更新到显存中
		 *	@return		int
		 **/
		virtual int update() = 0;

		/**
		 *	@name		getDsplModel
		 *	@brief		获取Element类型对应的显示Model对象
		 *	@return		zRender::ElemDsplModel<zRender::BasicEffect>* 显示Model对象指针
		 **/
		virtual ElemDsplModel<BasicEffect>* getDsplModel() const = 0;
	};
}


#endif//_ZRENDER_ELEMENT_META_DATA_H_
