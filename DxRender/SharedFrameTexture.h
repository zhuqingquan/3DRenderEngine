/**
 *	@date		2017-07-25
 *	@name		SharedFrameTexture.h
 *	@brief		Shared ID3D11Texture2D objs
 */

#pragma once
#ifndef _Z_RENDER_SHARED_FRAME_TEXTURE_H_
#define _Z_RENDER_SHARED_FRAME_TEXTURE_H_

#include "DxRenderCommon.h"
#include <d3d9.h>
#include <D3D11.h>
#include <string>

namespace zRender
{
	class SharedResource
	{
	public:
		SharedResource(HANDLE hRes, REFIID resIID);
		SharedResource();
		~SharedResource();

		//miplevels = 1, Stage, CPU_ACCESS_WRITE
		int create(ID3D11Device* pSrcDevice, ID3D11DeviceContext* devContex, DXGI_FORMAT fmt, int width, int height, int usage, const unsigned char* initData, int dataLen);
		int open(ID3D11Device* pOtherDevice, void** ppOpendSharedResource, IDXGIKeyedMutex** keyMutex);
		int open(IDirect3DDevice9Ex* pOtherDevice, void** ppOpendSharedResource);
		int update(ID3D11Device* pSrcDevice, ID3D11DeviceContext* devContex, int width, int height, const unsigned char* initData, int dataLen, int pitch);

		HANDLE getSharedHandle() const { return m_hRes; }
		ID3D11Texture2D* getTextureRes() const { return m_textureRes; }
		REFIID getTextureTypeID() const;
		DXGI_FORMAT getTextureFmt() const { return m_fmt; }
		//当create调用了，返回的字符串不为NULL。如果是Open的资源则返回的字符串为"".
		const std::wstring& getAdapterName() const { return m_strAdapter; }

		DWORD AcquireSync(DWORD timeout);
		DWORD ReleaseSync();
	private:
		void getAdapterName(ID3D11Device* pSrcDevice);

		HANDLE m_hRes;
		REFIID m_resIID;
		DXGI_FORMAT m_fmt;
		std::wstring m_strAdapter;
		//create接口中传入的用于创建Texture的Device，这个Device与open接口中的Device可能不是同一个，所以该共享显存的使用者应该不能直接使用这个Texture
		ID3D11Texture2D* m_textureRes;
		IDXGIKeyedMutex* m_resMutex;
		UINT acqKey;
		UINT relKey;
	};//class SharedFrameTexture

	class SharedTexture
	{
	public:
		virtual ~SharedTexture() = 0 {};
		/**
		 *	@name		create
		 *	@brief		创建可共享的Texture，由源数据的生成者负责创建这个Texture
		 *				调用该方法成功后，用户可通过getSharedResource接口获取到该对象中包含的共享Texture资源
		 *				在SharedResource类中提供接口用于open共享的Texture
		 *	@param[in]	PIXFormat fmt Texture的像素格式，该参数决定了SharedTexture内包含的SharedResource对象个数
		 *	@param[in]	int width	图片的宽
		 *	@param[in]	int height  图片的高
		 *	@param[in]	const char* initData 初始化的数据，NULL表示不在创建时初始化，否则提供图片像素数据初始化Texture
		 *	@param[in]	int dataLen 图片像素数据的长度，字节数。该长度必须大于或等于FRAMESIZE(width, height, pixfmt)
		 *	@return		int 0--创建成功	<0--创建失败
		 **/
		virtual int create(PIXFormat fmt, int width, int height,
							const unsigned char* initData, int dataLen, int yPitch, int uvPitch) = 0;

		/**
		 *	@name		release
		 *	@brief		释放create方法中创建的资源
		 *	@return		int 0--成功	<0--失败
		 **/
		virtual int release() = 0;

		virtual int update(PIXFormat fmt, int width, int height,
			const unsigned char* initData, int dataLen, int yPitch, int uvPitch) = 0;
		
		/**
		 *	@name		getSharedResource
		 *	@brief		获取在create方法中创建的SharedResource对象队列
		 *				用户可使用该队列，不需负责释放，该对象在release之后不再可用
		 *	@param[out]	SharedResource** ppSharedRes 保存SharedResource对象队列指针
		 *	@param[out]	int& count 获取SharedResource对象队列的个数
		 *
		 *	@return		int SharedResource队列中对象个数
		 **/
		virtual int getSharedResource(SharedResource** ppSharedRes, int& count) = 0;

		virtual bool getDescription(PIXFormat& fmt, int& width, int& height) = 0;
	};

	class SharedTexture_ARGB8 : public SharedTexture
	{
	public:
		SharedTexture_ARGB8(ID3D11Device* pSrcDevice, ID3D11DeviceContext* devContex);
		~SharedTexture_ARGB8();

		virtual int create(PIXFormat fmt, int width, int height,
						const unsigned char* initData, int dataLen, int yPitch, int uvPitch);
		virtual int release();
		virtual int update(PIXFormat fmt, int width, int height,
			const unsigned char* initData, int dataLen, int yPitch, int uvPitch);
		virtual int getSharedResource(SharedResource** ppSharedRes, int& count);
		virtual bool getDescription(PIXFormat& fmt, int& width, int& height);
	private:
		SharedResource* m_resARGB;
	};

	class SharedTexture_I420P : public SharedTexture
	{
	public:
		SharedTexture_I420P(ID3D11Device* pSrcDevice, ID3D11DeviceContext* devContex);
		~SharedTexture_I420P();

		virtual int create(PIXFormat fmt, int width, int height,
							const unsigned char* initData, int dataLen, int yPitch, int uvPitch);
		virtual int release();
		virtual int update(PIXFormat fmt, int width, int height,
			const unsigned char* initData, int dataLen, int yPitch, int uvPitch);
		virtual int getSharedResource(SharedResource** ppSharedRes, int& count);
		virtual bool getDescription(PIXFormat& fmt, int& width, int& height);
	private:
		SharedResource* m_resList[3];
		int m_resCount;

		ID3D11Device* m_pSrcDevice;
		ID3D11DeviceContext* m_devContext;

		PIXFormat m_pixfmt;
		int m_width;
		int m_height;
	};
}

#endif //_Z_RENDER_SHARED_FRAME_TEXTURE_H_