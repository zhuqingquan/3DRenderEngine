/** 
 *	@file		BackbufferRT.h
 *	@author		zhuqingquan
 *	@brief		Create SwapChain and Backbuffer as render target.
 *	@created	2019/05/12
 **/
#pragma once
#ifndef _DX_RENDER_BACKBUFFER_RT_H_
#define _DX_RENDER_BACKBUFFER_RT_H_

#include <d3d11.h>

namespace zRender
{
	class BackbufferRenderTarget
	{
	public:
		BackbufferRenderTarget();
		~BackbufferRenderTarget();

		int Initialize(ID3D11Device* device, HWND hwnd, DXGI_FORMAT fmt,
			bool isEnable4XMSAA = false, bool isGDICompatible = false );
		void Shutdown();
		int Resize(int newWidth, int newHeiht, ID3D11Device* device);

		void SetRenderTarget(ID3D11DeviceContext* ctx);
		void ClearRenderTarget(ID3D11DeviceContext* ctx, float red, float green, float blue, float alpha);
		//ID3D11ShaderResourceView* GetShaderResourceView();
		IDXGISwapChain* getSwapchain() const { return m_swapchain; }
		ID3D11Texture2D* getBackbuffer() const { return m_backbuffer; }

		int GetWidth() const { return m_width; }
		int GetHeight() const { return m_height; }
		DXGI_FORMAT GetPixelFormat() const { return m_fmt; }
		HWND GetHwnd() const { return m_hwnd; }
		ID3D11Texture2D* getRenderTargetTexture() const { return getBackbuffer(); }

		int LockBackbufferHDC(BOOL Discard, HDC* outHDC);
		int unlockBackbufferHDC(HDC hdc);
	private:
		BackbufferRenderTarget(const BackbufferRenderTarget&);

		bool CheckMultisampleQualityLevels(DXGI_FORMAT fmt, ID3D11Device* device);
		bool CreateDepthBufferAndView(int width, int height, ID3D11Device* device);

		IDXGISwapChain*				m_swapchain;
		ID3D11Texture2D*			m_backbuffer;
		ID3D11RenderTargetView*		m_renderTargetView;
		//ID3D11ShaderResourceView*	m_shaderResourceView;
		ID3D11Texture2D*			m_depthBuffer;
		ID3D11DepthStencilView*		m_depthView;
		HWND m_hwnd;
		int m_width;
		int m_height;
		DXGI_FORMAT m_fmt;
		bool m_bEnable4xMsaa;
		UINT m_i4xMsaaQuality;

		IDXGISurface1* m_bkbufDxgiSurface;				// 用于临时保存LockBackbufferHDC数据
	};
}

#endif//_DX_RENDER_BACKBUFFER_RT_H_