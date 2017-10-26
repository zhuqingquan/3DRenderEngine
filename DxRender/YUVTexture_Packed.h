/**
 *	@author		zhuqingquan
 *	@date		2014-11-19
 *	@name		YUVTexture_Packed.h
 *	@brief		紧凑型内存布局的YUV像素类型的图片在显卡中显示所需的Texture资源类，先只支持PIXFMT_YUY2类型
 */

#pragma once
#ifndef _ZRENDER_YUVTEXTURE_PACKED_H_
#define _ZRENDER_YUVTEXTURE_PACKED_H_

#include "DxZRenderDLLDefine.h"
#include "IDisplayContentProvider.h"
#include "IRawFrameTexture.h"

#pragma warning(push)
#pragma warning(disable:4251)

namespace zRender
{
	/**
	 *	@name	YUVTexture_Packed
	 *	@brief	紧凑型内存布局的YUV像素类型的图片在显卡中显示所需的Texture资源类，先只支持PIXFMT_YUY2类型
	 **/
	class DX_ZRENDER_EXPORT_IMPORT YUVTexture_Packed : public IRawFrameTexture
	{
	public:
		/**
		 *	@name		YUVTexture_Packed
		 *	@brief		构造函数，只是构造类对象，没有在此创建Texture资源
		 *				构造对象成功后还需调用create接口创建Texture资源
		 *	@param[in]	PIXFormat pixfmt 紧凑型内存布局的YUV图片的数据内存存放的格式，必须是PIXFMT_YUY2，其他参数将导致调用create失败
		 **/
		YUVTexture_Packed(PIXFormat pixfmt);

		/**
		 *	@name		~YUVTexture_Packed
		 *	@brief		析构函数，释放已经创建的Texture资源
		 **/
		~YUVTexture_Packed();

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
		 *				这些数据将被认为是像素类型与该Texture一样的数据（紧凑型的YUV数据，如YUY2）。因此dataLen必须>=FRAMESIZE(width, height, pixfmt)
		 *	@param[in]	int dataLen	pData指向的数据的长度。必须>=FRAMESIZE(width, height, pixfmt)
		 *	@param[in]	int yPitch	图片数据的行宽，即保存一行像素所需的字节数。必须>=FRAMEPITCH(width, pixfmt)
		 *	@param[in]	int uPitch	图片数据的行宽，在YUY2格式中该参数无效
		 *	@param[in]	int vPitch	图片数据的行宽，在YUY2格式中该参数无效
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
		YUVTexture_Packed(const YUVTexture_Packed&);
		YUVTexture_Packed& operator=(const YUVTexture_Packed&);

		struct FrameTexture
		{
			ID3D11Texture2D* m_yuvTexStage;

			ID3D11Texture2D* m_yuvTex;
			ID3D11Texture2D* m_parityTex;
			ID3D11ShaderResourceView*	m_yuvSRV;
			ID3D11ShaderResourceView*	m_paritySRV;
			int m_width;
			int m_height;
			PIXFormat m_pixfmt;

			FrameTexture() 
				: m_width(0), m_height(0)
				, m_yuvTexStage(NULL)
				, m_yuvTex(NULL), m_parityTex(NULL)
				, m_yuvSRV(NULL), m_paritySRV(NULL)
			{

			}

			void destroy();

			int update(const unsigned char* pData, int dataLen, int yPitch, int uPitch, int vPitch, int width, int height,
				const RECT& regionUpdated, ID3D11DeviceContext* d3dDevContex);
			int getTexture(ID3D11Texture2D** outYUVTexs, int& texsCount) const;
			int getShaderResourceView(ID3D11ShaderResourceView** outYUVSRVs, int& srvsCount) const;
			bool valid() const;
		};
		FrameTexture m_VideoFrame;

		FrameTexture create_txframe(ID3D11Device* device, int width, int height,
			const char* initData, int dataLen);
	};
}

#pragma warning(pop)

#endif ///_ZRENDER_YUVTEXTURE_PACKED_H_