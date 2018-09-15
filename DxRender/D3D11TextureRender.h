/**
*	@file		D3D11TextureRender.h
*	@brief		实现将Texture直接按照像素坐标渲染
*				可以用于简单的图片渲染和图片伸缩
*	@author		zhuqingquan
**/
#pragma once
#ifndef _Z_RENDER_D3D11_TEXTURE_RENDER_H_
#define _Z_RENDER_D3D11_TEXTURE_RENDER_H_

#include <D3D11.h>

namespace zRender
{
	class D3D11TextureRender
	{
	public:
		D3D11TextureRender(ID3D11Device* device, ID3D11DeviceContext* context);
		~D3D11TextureRender();

		int draw(ID3D11Texture2D* texture, ID3D11ShaderResourceView* shaderResView, const RECT& rect);
		int draw(ID3D11Texture2D* texture, ID3D11ShaderResourceView* shaderResView, const RECT& rect, UINT uNumRtt, ID3D11RenderTargetView** ppRttArray);

	private:
		bool setupPipeline(int rttViewCount, ID3D11RenderTargetView** ppRTTView, const RECT& rect, ID3D11ShaderResourceView* shaderResView);

		bool createResource();
		void releaseResource();

		bool createPixelShader();
		void releasePixelShader();
		bool createVertexShader();
		void releaseVertexShader();
		bool createVertexBuffer();
		void releaseVertexBuffer();
		bool createBlendState();
		void releaseBlendState();
		bool createDepthStencilState();
		void releaseDepthStencilState();
		bool createSamplerState();
		void releaseSamplerState();
		bool createRasterizerState();
		void releaseRasterizerState();

		bool createRenderTargetViewForBackbuffer();
		void releaseBackbufferRenderTargetView();
	private:
		ID3D11Device*			m_device; 
		ID3D11DeviceContext*	m_context;

		ID3D11PixelShader*		m_pixshader;
		ID3D11Buffer*			m_vertexBuf;
		ID3D11VertexShader*		m_vertexShader;
		ID3D11InputLayout*		m_vertexLayout;
		ID3D11BlendState*		m_blendState;
		ID3D11DepthStencilState*	m_depthState;
		ID3D11SamplerState*		m_samplerState;
		ID3D11RasterizerState*	m_rasterizerState;
		ID3D11RenderTargetView*	m_backbufferRTTView;

		bool					m_initSuccess;
	};//class D3D11TextureRender
}

#endif //_Z_RENDER_D3D11_TEXTURE_RENDER_H_
