/**
 *	@author		zhuqingquan
 *	@date		2014-11-19
 *	@name		YUVTexture_Packed.h
 *	@brief		NV12类型的YUV像素类型的图片在显卡中显示所需的Texture资源类，先只支持PIXFMT_NV12类型
 */

#pragma once
#ifndef _ZRENDER_YUVTEXTURE_NV12_H_
#define _ZRENDER_YUVTEXTURE_NV12_H_

#include "IDisplayContentProvider.h"
#include "IRawFrameTexture.h"

namespace zRender
{
	class YUVTexture_NV12 : public IRawFrameTexture
	{
	public:
		/**
		 *	@name		YUVTexture_NV12
		 *	@brief		构造函数，只是构造类对象，没有在此创建Texture资源
		 *				构造对象成功后还需调用create接口创建Texture资源
		 *	@param[in]	PIXFormat pixfmt NV12内存布局的YUV图片的数据内存存放的格式，必须是PIXFMT_NV12，其他参数将导致调用create失败
		 **/
		YUVTexture_NV12(PIXFormat pixfmt);

		/**
		 *	@name		~YUVTexture_NV12
		 *	@brief		析构函数，释放已经创建的Texture资源
		 **/
		~YUVTexture_NV12();

		/**
		 *	@name		create
		 *	@brief		参见基类接口 @ref IRawFrameTexture 的定义说明
		 **/
		int create(ID3D11Device* device, int width, int height,
					const char* initData, int dataLen);

		/**
		 *	@name		destroy
		 *	@brief		参见基类接口 @ref IRawFrameTexture 的定义说明
		 **/
		int destroy();

		/**
		 *	@name		getTexture
		 *	@brief		参见基类接口 @ref IRawFrameTexture 的定义说明
		 **/
		int getTexture(ID3D11Texture2D** outYUVTexs, int& texsCount) const;

		/**
		 *	@name		getShaderResourceView
		 *	@brief		参见基类接口 @ref IRawFrameTexture 的定义说明
		 **/
		int getShaderResourceView(ID3D11ShaderResourceView** outYUVSRVs, int& srvsCount) const;

		/**
		 *	@name		update
		 *	@brief		使用pData指向的内存中的数据更新Texture，数据将被拷贝到Texture中覆盖Texture原有的数据
		 *	@param[in]	const unsigned char* pData 需要更新拷贝到Texture中的数据的内存地址
		 *				这些数据将被认为是像素类型与该Texture一样的数据（NV12）。因此dataLen必须>=FRAMESIZE(width, height, pixfmt)
		 *	@param[in]	int dataLen	pData指向的数据的长度。必须>=FRAMESIZE(width, height, pixfmt)
		 *	@param[in]	int yPitch	图片数据的行宽，即保存一行像素所需的字节数。必须>=FRAMEPITCH(width, pixfmt)
		 *	@param[in]	int uPitch	图片数据的行宽，uv数据的每行步长
		 *	@param[in]	int vPitch	图片数据的行宽，该数据在NV12格式数据中无效
		 *	@param[in]	int width	图片的宽,必须>0
		 *	@param[in]	int height  图片的高,必须>0
		 *	@param[in]	const RECT& regionUpdated 图片中数据将被拷贝的区域，该区域的数据将拷贝到Texture的左上角起点位置
		 *	@param[in]	ID3D11DeviceContext* d3dDevContex 与该Texture所属的ID3D11Device对象对应的ID3D11DeviceContext对象
		 *	@return		int 0--创建成功	<0--创建失败
		 **/
		int update(const unsigned char* pData, int dataLen, int yPitch, int uPitch, int vPitch, int width, int height,
							const RECT& regionUpdated, ID3D11DeviceContext* d3dDevContex);

		virtual int update(SharedTexture* pSharedTexture, const RECT& regionUpdated, ID3D11DeviceContext* d3dDevContex);
	private:
		YUVTexture_NV12(const YUVTexture_NV12&);
		YUVTexture_NV12& operator=(const YUVTexture_NV12&);

		ID3D11Texture2D* m_yTex;
		ID3D11Texture2D* m_uvTex;
		ID3D11ShaderResourceView*	m_ySRV;
		ID3D11ShaderResourceView*	m_uvSRV;
	};
}

#endif //_ZRENDER_YUVTEXTURE_NV12_H_