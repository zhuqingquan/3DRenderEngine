/**
 *	@author		zhuqingquan
 *	@date		2014-10-28
 *	@name		IDisplayContentProvider.h
 *	@brief		显示内容的提供者的接口定义，定义供外部获取显示渲染所需的顶点信息、纹理Texture、Shader数据等信息的接口
 */

#pragma once
#ifndef _ZRENDER_IDISPLAYCONTENTPROVIDER_H_
#define _ZRENDER_IDISPLAYCONTENTPROVIDER_H_

#include "Vertex.h"
#include "DxRenderCommon.h"
#include "DxZRenderDLLDefine.h"

namespace zRender
{
	class SharedTexture;
	class IRawFrameTexture;

	/**
	 *	@name		TextureDataSource
	 *	@brief		纹理数据获取、更新的接口定义
	 **/
	class DX_ZRENDER_EXPORT_IMPORT TextureDataSource
	{
	public:
		/**
		 *	@name		TextureDataSource
		 *	@brief		构造函数
		 **/
		TextureDataSource() {};

		/**
		 *	@name		~TextureDataSource
		 *	@brief		析构方法，纯虚方法，可继承
		 **/
		virtual ~TextureDataSource() = 0 {};

		/**
		 *	@name		isUpdated
		 *	@brief		获取当前纹理数据是否已更新，用户可以根据该方法返回值判断是否需要拷贝纹理数据
		 *	@param[in]	int identify 用户当前所持有的纹理数据的标识符，标识用户当前使用的纹理数据
		 *	@return		bool true--纹理数据已更新	false--纹理数据为更新
		 **/
		virtual bool isUpdated(int identify) const = 0;

		/**
		 *	@name		getTextureProfile
		 *	@brief		获取纹理数据的相关信息，如数据的长度，数据的行宽，以及高
		 *				这里获取的是纹理中由textureReg指定的区域中的数据信息
		 *	@param[in]	const RECT_f& textureReg 需要获取的纹理的区域，该参数表示用户想要获取纹理部分区域的信息。
		 *				该参数为相对坐标，取值范围为[0,1]
		 *				整个纹理作为一个整体，如果该参数为[0, 0, 0.5, 0.5]则获取的信息为Texture左上角四分之一部分的宽高等。
		 *	@param[out]	int& dataLen 传出数据的长度
		 *	@param[out]	int& yPitch 传出纹理数据的行宽，Y数据或者RGB数据、YUY2数据的行宽
		 *	@param[out]	int& uPitch 传出纹理数据的行宽，U数据或者UV、VU数据的行宽
		 *	@param[out]	int& vPitch 传出纹理数据的行宽，V数据的行宽
		 *	@param[out]	int& width 传出纹理的像素宽
		 *	@param[out]	int& height 传出纹理的高
		 *	@param[out]	int& pixelFmt 传出纹理的像素格式
		 *	@return		int 0--获取成功  <0--失败，可能是成员变量未初始化或者成员变量值不合法
		 **/
		virtual int getTextureProfile(const RECT_f& textureReg, int& dataLen, int& yPitch, int& uPitch, int& vPitch, int& width, int& height, PIXFormat& pixelFmt) = 0;

		/**
		 *	@name		getData
		 *	@brief		获取纹理数据的相关信息，如数据的首地址，数据的长度，数据的行宽，以及高
		 *				这里获取的是完整的纹理数据，而不是纹理的一部分
		 *	@param[out]	int& dataLen 传出数据的长度
		 *	@param[out]	int& yPitch 传出纹理数据的行宽，Y数据或者RGB数据、YUY2数据的行宽
		 *	@param[out]	int& uPitch 传出纹理数据的行宽，U数据或者UV、VU数据的行宽
		 *	@param[out]	int& vPitch 传出纹理数据的行宽，V数据的行宽
		 *	@param[out]	int& width 传出纹理的像素宽
		 *	@param[out]	int& height 传出纹理的高
		 *	@param[out]	PIXFormat& pixelFmt 传出纹理的像素格式
		 *	@return		unsigned char* 非NULL--纹理数据的首地址	 NULL--无数据
		 **/
		virtual unsigned char* getData(int& dataLen, int& yPitch, int& uPitch, int& vPitch, int& width, int& height, PIXFormat& pixelFmt, RECT& effectReg, int& identify) = 0;
		
		/**
		 *	@name		getSharedTexture
		 *	@brief		获取共享显存对象
		 *	@return		SharedTexture* 非NULL--共享显存对象	 NULL--无数据
		 **/
		virtual SharedTexture* getSharedTexture(RECT& effectReg, int& identify) = 0;

		virtual IRawFrameTexture* getTexture() = 0;

		/**
		 *	@name		copyDataToTexture
		 *	@brief		将数据拷贝到D3D11中的Texture对象中。
		 *	@param[in]	const RECT_f& textureReg 需要拷贝的纹理的区域，该参数表示用户想要拷贝纹理部分区域的数据。
		 *				该参数为相对坐标，取值范围为[0,1]
		 *				整个纹理作为一个整体，如果该参数为[0, 0, 0.5, 0.5]则拷贝Texture左上角四分之一部分的数据。
		 *	@param[in]	unsigned char* dstTextureData Texture对象对应的显存资源的首地址
		 *	@param[in]	int pitch Texture对象对应的显存资源的行宽
		 *	@param[in]	int height Texture对象对应的显存资源的高
		 *	@param[out]	int& identify 如果成功则传出本次拷贝的Texture数据的标识符，这样下次可以通过这个标识符查询是否Texture被更新了
		 *	@return		int 0--拷贝成功	 <0--拷贝失败  >0--未更新，不需拷贝
		 **/
		virtual int copyDataToTexture(const RECT_f& textureReg, unsigned char* dstTextureData, int pitch, int height, int& identify) = 0;

		virtual void increaseAuthorization() {};
		virtual void decreaseAuthorization() {};
	};

	/**
	 *	@name		IDisplayContentProvider
	 *	@brief		显示内容的提供者的接口定义，定义供外部获取显示渲染所需的顶点信息、纹理Texture、Shader数据等信息的接口
	 **/
	class DX_ZRENDER_EXPORT_IMPORT IDisplayContentProvider
	{
	public:
		/**
		 *	@name		IDisplayContentProvider
		 *	@brief		构造函数
		 **/
		IDisplayContentProvider() {};

		/**
		 *	@name		~IDisplayContentProvider
		 *	@brief		析构方法，纯虚方法，可继承
		 **/
		virtual ~IDisplayContentProvider() = 0
		{
		}

		/**
		 *	@name		isVertexUpdated
		 *	@brief		获取当前的顶点信息是否被更新
		 *				通过匹配参数identify与顶点信息的标示符来判断顶点信息是否被更新
		 *	@param[in]	int identify 顶点信息的标识符
		 *	@return		int 0--成功  <0--失败
		 **/
		virtual bool isVertexUpdated(int identify) const = 0;

		/**
		 *	@name		getVertexs
		 *	@brief		获取所有顶点信息。不同拓扑关系的顶点放入不同的VertexVector对象中。
		 *	@param[in,out]	VertexVector** vv 所有拓扑管理的顶点信息的VertexVector对象的数组，外部不需释放该数组，其内存由IDisplayContentProvider负责释放
		 *	@param[in,out] int& vvCount 所有拓扑管理的顶点信息的VertexVector对象的数组的长度
		 *	@param[in,out]	int& identify 顶点信息的标识符
		 *	@return		int 0--成功  <0--失败
		 **/
		virtual int getVertexs(VertexVector** vv, int& vvCount, int& identify) = 0;

		/**
		 *	@name		getTextureDataSource
		 *	@brief		获取纹理数据提供与更新的对象
		 *	@return		TextureDataSource* 非NULL--纹理数据提供与更新的对象  NULL--无纹理对象
		 **/
		virtual TextureDataSource* getTextureDataSource() = 0;

		/**
		 *	@name		getShader
		 *	@brief		获取渲染所需的Shader资源，该接口暂时不可用，返回值暂时未定义
		 *	@return		void* 暂时未定义
		 **/
		virtual void* getShader() = 0;

		virtual void increaseAuthorization() {};
		virtual void decreaseAuthorization() {};
	};
}//namespace zRender

#endif //_zRENDER_IDISPLAYCONTENTPROVIDER_H_