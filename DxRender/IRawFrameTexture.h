/**
 *	@author		zhuqingquan
 *	@date		2014-11-14
 *	@name		IRawFrameTexture.h
 *	@brief		原始数据图像显示所用的Texture接口定义，底层使用ID3D11Texture2D实现。图像数据都是解码后的像素值
 *				YUV图像、ARGB图像的显示可以继承该接口保存图像数据
 */

#pragma once
#ifndef _ZRENDER_IPICTURETEXTURE_H_
#define _ZRENDER_IPICTURETEXTURE_H_

#include <D3D11.h>
#include "DxRenderCommon.h"
#include "DxZRenderDLLDefine.h"

namespace zRender
{
	class SharedTexture;

	/**
	 * @name		IRawFrameTexture
	 * @brief		原始数据图像显示所用的Texture接口定义，底层使用ID3D11Texture2D实现。图像数据都是解码后的像素值
	 *				YUV图像、ARGB图像的显示可以继承该接口保存图像数据
	 **/
	class DX_ZRENDER_EXPORT_IMPORT IRawFrameTexture
	{
	public:
		/**
		 *  @name		IRawFrameTexture
		 *  @brief		构造函数，初始化宽、高为0，根据参数初始化图片的像素类型
		 *	@param[in]	PIXFormat pixfmt 图片的像素类型
		 **/
		IRawFrameTexture(PIXFormat pixfmt) 
			: m_width(0), m_height(0), m_device(0), m_pixfmt(pixfmt) {};

		/**
		 *	@name		~IRawFrameTexture
		 *	@brief		析构函数
		 **/
		virtual ~IRawFrameTexture() = 0 {};

		/**
		 *	@name		create
		 *	@brief		创建底层显示的ID3D11Texture2D用于保存图片数据与显示
		 *	@param[in]	ID3D11Device* device 创建的Texture资源所属的Device设备
		 *	@param[in]	int width	图片的宽
		 *	@param[in]	int height  图片的高
		 *	@param[in]	const char* initData 初始化的数据，NULL表示不在创建时初始化，否则提供图片像素数据初始化Texture
		 *	@param[in]	int dataLen 图片像素数据的长度，字节数。该长度必须大于或等于FRAMESIZE(width, height, pixfmt)
		 *	@return		int 0--创建成功	<0--创建失败
		 **/
		virtual int create(ID3D11Device* device, int width, int height,
							const char* initData, int dataLen) = 0;

		/**
		 *	@name		destroy
		 *	@brief		销毁create方法中所创建的资源
		 *	@return		int 0--成功	<0--失败
		 **/
		virtual int destroy() = 0;

		/**
		 *	@name		getTexture
		 *	@brief		获取已创建的底层显示的ID3D11Texture2D对象
		 *	@param[out]	ID3D11Texture2D** outTexs 保存所有创建的ID3D11Texture2D对象的指针数组
		 *	@param[in,out]	int& texsCount	作为传入参数表示指针数组outTexs中指针的个数， 作为传出参数保存有效的ID3D11Texture2D对象的个数
		 *	@return		int 0--成功	<0--失败 ， 当create失败或者destroy已经调用时，调用该接口也会是失败
		 **/
		virtual int getTexture(ID3D11Texture2D** outTexs, int& texsCount) const = 0;

		/**
		 *	@name		getTexture
		 *	@brief		获取已创建的底层显示的ID3D11Texture2D对象对应的ID3D11ShaderResourceView对象
		 *	@param[out]	ID3D11ShaderResourceView** outSRVs 保存所有创建的ID3D11ShaderResourceView对象的指针数组
		 *	@param[in,out]	int& srvsCount	作为传入参数表示指针数组outSRVs中指针的个数， 作为传出参数保存有效的ID3D11ShaderResourceView对象的个数
		 *	@return		int 0--成功	<0--失败 ， 当create失败或者destroy已经调用时，调用该接口也会是失败
		 **/
		virtual int getShaderResourceView(ID3D11ShaderResourceView** outSRVs, int& srvsCount) const = 0;

		/**
		 *	@name		update
		 *	@brief		使用pData指向的内存中的数据更新Texture，数据将被拷贝到Texture中覆盖Texture原有的数据
		 *	@param[in]	const unsigned char* pData 需要更新拷贝到Texture中的数据的内存地址
		 *				这些数据将被认为是像素类型与该Texture一样的数据。因此dataLen必须>=FRAMESIZE(width, height, pixfmt)
		 *	@param[in]	int dataLen	pData指向的数据的长度。必须>=FRAMESIZE(width, height, pixfmt)
		 *	@param[in]	int yPitch	图片数据的行宽，即保存一行像素所需的字节数。必须>=FRAMEPITCH(width, pixfmt)
		 *	@param[in]	int uPitch	图片数据的行宽，即保存一行像素所需的字节数。必须>=FRAMEPITCH(width, pixfmt)
		 *	@param[in]	int vPitch	图片数据的行宽，即保存一行像素所需的字节数。必须>=FRAMEPITCH(width, pixfmt)
		 *	@param[in]	int width	图片的宽,必须>0
		 *	@param[in]	int height  图片的高,必须>0
		 *	@param[in]	const RECT& regionUpdated 图片中数据将被拷贝的区域，该区域的数据将拷贝到Texture的左上角起点位置
		 *	@param[in]	ID3D11DeviceContext* d3dDevContex 与该Texture所属的ID3D11Device对象对应的ID3D11DeviceContext对象
		 *	@return		int 0--创建成功	<0--创建失败
		 **/
		virtual int update(const unsigned char* pData, int dataLen, int yPitch, int uPitch, int vPitch,  int width, int height,
							const RECT& regionUpdated, ID3D11DeviceContext* d3dDevContex) = 0;

		virtual int update(SharedTexture* pSharedTexture, const RECT& regionUpdated, ID3D11DeviceContext* d3dDevContex) = 0;

		/**
		 *	@name		getWidth
		 *	@brief		获取图片的宽，像素值
		 *	@return		int 图片的宽
		 **/
		int getWidth()  const { return m_width; }

		/**
		 *	@name		getHeight
		 *	@brief		获取图片的高，像素值
		 *	@return		int 图片的高
		 **/
		int getHeight() const { return m_height; }

		/**
		 *	@name		getParentDevice
		 *	@brief		获取该Texture资源所属的ID3D11Device对象，该对象在create调用成功后保存
		 *	@return		ID3D11Device* Texture资源所属的ID3D11Device对象
		 **/
		ID3D11Device* getParentDevice() const { return m_device; }

		/**
		 *	@name		getPixelFormat
		 *	@brief		获取图片解码后的像素类型
		 *	@return		PIXFormat 图片的像素类型
		 **/
		PIXFormat getPixelFormat() const { return m_pixfmt; }

	protected:
		PIXFormat m_pixfmt;
		int m_width;
		int m_height;
		ID3D11Device* m_device;

	private:
		IRawFrameTexture(const IRawFrameTexture&);
		IRawFrameTexture& operator=(const IRawFrameTexture&);
	};
}

#endif //_zRENDER_IPICTURETEXTURE_H_