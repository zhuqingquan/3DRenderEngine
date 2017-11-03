/**
 *	@author		zhuqingquan
 *	@date		2014-11-19
 *	@name		YUVTexture_Planar.h
 *	@brief		平面型内存布局的YUV像素类型的图片在显卡中显示所需的Texture资源类，先只支持PIXFMT_YUV420P、PIXFMT_YV12类型
 */


#pragma once
#ifndef _ZRENDER_YUVTEXTURE_H_
#define _ZRENDER_YUVTEXTURE_H_

#include <vector>
#include "DxRenderCommon.h"
#include "inc/RawFrameTextureBase.h"

namespace zRender
{
	/**
	 *	@name	YUVTexture_Planar
	 *	@brief	YUV图片在显卡中显示所需的Texture资源类，只能用于平面型数据内存布局格式的YUV图片，包括YUV420P（I420）、YV12
	 **/
	class YUVTexture_Planar : public RawFrameTextureBase
	{
	public:
		/**
		 *	@name		YUVTexture_Planar
		 *	@brief		构造函数，只是构造类对象，没有在此创建Texture资源
		 *				构造对象成功后还需调用create接口创建Texture资源
		 *	@param[in]	PIXFormat yuvFmt YUV图片的数据内存存放的格式，必须是PIXFMT_YUV420P或者PIXFMT_YV12，其他参数将导致调用create失败
		 **/
		YUVTexture_Planar(PIXFormat yuvFmt);

		/**
		 *	@name		~YUVTexture_Planar
		 *	@brief		析构函数，释放已经创建的Texture资源
		 **/
		~YUVTexture_Planar();

		/**
		 *	@name		create
		 *	@brief		参见基类接口 @ref IRawFrameTexture 的定义说明
		 **/
		int create(ID3D11Device* device, int width, int height,
					const char* initData, int dataLen);

		virtual int create(ID3D11Device* device, int width, int height, TEXTURE_USAGE usage, bool bShared, const char* initData, int dataLen, int pitch);

		/**
		 *	@name		destroy
		 *	@brief		参见基类接口 @ref IRawFrameTexture 的定义说明
		 **/
		int destroy();

		/**
		 *	@name		update
		 *	@brief		使用pData指向的内存中的数据更新Texture，数据将被拷贝到Texture中覆盖Texture原有的数据
		 *	@param[in]	const unsigned char* pData 需要更新拷贝到Texture中的数据的内存地址
		 *				这些数据将被认为是像素类型与该Texture一样的数据（如YUV420P、YV12）。因此dataLen必须>=FRAMESIZE(width, height, pixfmt)
		 *	@param[in]	int dataLen	pData指向的数据的长度。必须>=FRAMESIZE(width, height, pixfmt)
		 *	@param[in]	int yPitch	因为是YUV平面布局，Y每行数据的的行宽
		 *	@param[in]	int uPitch	因为是YUV平面布局，U每行数据的的行宽
		 *	@param[in]	int vPitch	因为是YUV平面布局，V每行数据的的行宽
		 *	@param[in]	int width	图片的宽,必须>0
		 *	@param[in]	int height  图片的高,必须>0
		 *	@param[in]	const RECT& regionUpdated 图片中数据将被拷贝的区域，该区域的数据将拷贝到Texture的左上角起点位置
		 *	@param[in]	ID3D11DeviceContext* d3dDevContex 与该Texture所属的ID3D11Device对象对应的ID3D11DeviceContext对象
		 *	@return		int 0--创建成功	<0--创建失败
		 **/
		int update(const unsigned char* pData, int dataLen, int yPitch, int uPitch, int vPitch, int width, int height,
							const RECT& regionUpdated, ID3D11DeviceContext* d3dDevContex);

		virtual int update(SharedTexture* pSharedTexture, const RECT& regionUpdated, ID3D11DeviceContext* d3dDevContex);

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
	private:
		YUVTexture_Planar(const YUVTexture_Planar&);
		YUVTexture_Planar& operator=(const YUVTexture_Planar&);

		struct FrameTexture
		{
			ID3D11Texture2D* m_yTexStage;
			ID3D11Texture2D* m_uTexStage;
			ID3D11Texture2D* m_vTexStage;

			ID3D11Texture2D* m_yTex;
			ID3D11Texture2D* m_uTex;
			ID3D11Texture2D* m_vTex;
			ID3D11ShaderResourceView*	m_ySRV;
			ID3D11ShaderResourceView*	m_uSRV;
			ID3D11ShaderResourceView*	m_vSRV;
			int m_width;
			int m_height;
			PIXFormat m_pixfmt;

			FrameTexture() 
				: m_width(0), m_height(0)
				, m_yTex(NULL), m_uTex(NULL), m_vTex(NULL)
				, m_ySRV(NULL), m_uSRV(NULL), m_vSRV(NULL)
				, m_yTexStage(NULL), m_uTexStage(NULL), m_vTexStage(NULL)
			{

			}

			void destroy();

			int update(const unsigned char* pData, int dataLen, int yPitch, int uPitch, int vPitch, int width, int height,
				const RECT& regionUpdated, ID3D11DeviceContext* d3dDevContex);
			virtual int update(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDevContex, SharedTexture* pSharedTexture, const RECT& regionUpdated);
			int getTexture(ID3D11Texture2D** outYUVTexs, int& texsCount) const;
			int getShaderResourceView(ID3D11ShaderResourceView** outYUVSRVs, int& srvsCount) const;
			bool valid() const;
		};
		FrameTexture m_VideoFrame;

		FrameTexture create_txframe(ID3D11Device* device, int width, int height,
							const char* initData, int dataLen);
	};
}

#endif //_ZRENDER_YUVTEXTURE_H_