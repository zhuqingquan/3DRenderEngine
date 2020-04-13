#include "BackbufferRT.h"
#include "DxRenderCommon.h"
#include "DXLogger.h"
#include "ConstDefine.h"
#include <tchar.h>

#define LOG_TAG L"BackbufferRenderTarget"

namespace zRender
{
	BackbufferRenderTarget::BackbufferRenderTarget()
		: m_hwnd(nullptr), m_width(0), m_height(0), m_fmt(DXGI_FORMAT_UNKNOWN)
		, m_swapchain(nullptr), m_backbuffer(nullptr)
		, m_renderTargetView(nullptr), m_depthBuffer(nullptr), m_depthView(nullptr)
		, m_bkbufDxgiSurface(nullptr)
		, m_bEnable4xMsaa(false), m_i4xMsaaQuality(0)
	{
	}

	BackbufferRenderTarget::~BackbufferRenderTarget()
	{
		Shutdown();
	}

	int BackbufferRenderTarget::Initialize(ID3D11Device * device, HWND hwnd, DXGI_FORMAT fmt,
		bool isEnable4XMSAA/* = false*/, bool isGDICompatible/* = false*/)
	{
		if (nullptr == device || nullptr == hwnd)
			return -1;
		RECT winRect = { 0, 0, 0, 0 };
		/*GetWindowRect*/GetClientRect(hwnd, &winRect);
		int winWidth = winRect.right - winRect.left;
		int winHeight = winRect.bottom - winRect.top;

		// 先检测MultisampleQualityLevel，获取level值
		m_bEnable4xMsaa = isEnable4XMSAA;
		if (m_bEnable4xMsaa && CheckMultisampleQualityLevels(fmt, device))
		{
			return -10;
		}

		DXGI_SWAP_CHAIN_DESC sd;
		sd.BufferDesc.Width = winWidth;
		sd.BufferDesc.Height = winHeight;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferDesc.Format = fmt;//DXGI_FORMAT_B8G8R8A8_UNORM;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;
		sd.OutputWindow = hwnd;
		sd.Windowed = true;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		sd.Flags = 0;
		if (isGDICompatible)
		{
			// 如果启用GDI compatible，则参数fmt设置的像素格式未必有效
			sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			sd.Flags |= DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;
		}
		// Use 4X MSAA? 
		if (isEnable4XMSAA)
		{
			sd.SampleDesc.Count = 4;
			sd.SampleDesc.Quality = m_i4xMsaaQuality - 1;
		}
		// No MSAA
		else
		{
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
		}

		// To correctly create the swap chain, we must use the IDXGIFactory that was
		// used to create the device.  If we tried to use a different IDXGIFactory instance
		// (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
		// This function is being called with a device from a different IDXGIFactory."
		IDXGIDevice1* dxgiDevice = 0;
		if (S_OK != device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgiDevice))
		{
 			TCHAR errmsg[512] = { 0 };
 			swprintf_s(errmsg, 512, L"Initialize: QueryInterface IDXGIDevice failed.");
 			log_e(LOG_TAG, errmsg);
			return -2;
		}
		IDXGIAdapter1* dxgiAdapter = 0;
		if (S_OK != dxgiDevice->GetParent(__uuidof(IDXGIAdapter1), (void**)&dxgiAdapter))
		{
 			log(LOG_TAG, LL_ERROR, L"Initialize: QueryInterface IDXGIAdapter failed.");
			return -3;
		}
		IDXGIFactory1* dxgiFactory = 0;
		if (S_OK != dxgiAdapter->GetParent(__uuidof(IDXGIFactory1), (void**)&dxgiFactory))
		{
 			TCHAR errmsg[512] = { 0 };
 			swprintf_s(errmsg, 512, L"Initialize: QueryInterface IDXGIFactory failed.");
 			log_e(LOG_TAG, errmsg);
			return -4;
		}

		IDXGISwapChain* swapChain = nullptr;
		HRESULT rslt = S_FALSE;
		if (S_OK != (rslt = dxgiFactory->CreateSwapChain(device, &sd, &swapChain)))
		{
// #ifdef _DEBUG
// 			printf("Error in DxRender_D3D11::init : Create Swap Chain failed.(Width=%d, Height=%d, SampleCount=%d, SampleQuality=%d)\n",
// 				sd.BufferDesc.Width, sd.BufferDesc.Height, sd.SampleDesc.Count, sd.SampleDesc.Quality);
// #endif
// 			TCHAR errmsg[1024] = { 0 };
// 			swprintf_s(errmsg, 1024, L"Error in DxRender_D3D11::init : Create Swap Chain failed.ERR:%s (Width=%d, Height=%d, Format=%d Flags=%d SampleCount=%d, SampleQuality=%d)\n",
// 				DXGetErrorDescription(rslt), sd.BufferDesc.Width, sd.BufferDesc.Height, sd.BufferDesc.Format, sd.Flags, sd.SampleDesc.Count, sd.SampleDesc.Quality);
// 			log_e(LOG_TAG, errmsg);
// 			return -8;
			return -5;
		}

		ReleaseCOM(dxgiDevice);
		ReleaseCOM(dxgiAdapter);
		ReleaseCOM(dxgiFactory);

		ID3D11Texture2D* backBuffer;
		if (S_OK != swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)))
			return -6;
		ID3D11RenderTargetView* renderTargetView = nullptr;
		if (S_OK != device->CreateRenderTargetView(backBuffer, 0, &renderTargetView))
		{
			ReleaseCOM(swapChain);
			ReleaseCOM(backBuffer);
// 			TCHAR errmsg[512] = { 0 };
// 			swprintf_s(errmsg, 512, L"Error in DxRender_D3D11::init : CreateRenderTargetView failed.");
// 			log_e(LOG_TAG, errmsg);
			return -7;
		}

		if (0 != CreateDepthBufferAndView(winWidth, winHeight, device))
		{
			ReleaseCOM(renderTargetView);
			ReleaseCOM(swapChain);
			ReleaseCOM(backBuffer);
			return -8;
		}

		m_fmt = sd.BufferDesc.Format;
		m_hwnd = hwnd;
		m_width = winWidth;
		m_height = winHeight;
		m_renderTargetView = renderTargetView;
		m_swapchain = swapChain;
		m_backbuffer = backBuffer;
		return 0;
	}

	void BackbufferRenderTarget::Shutdown()
	{
		ReleaseCOM(m_depthView);
		ReleaseCOM(m_depthBuffer);
		ReleaseCOM(m_renderTargetView);
		ReleaseCOM(m_backbuffer);
		ReleaseCOM(m_swapchain);
		m_width = m_height = 0;
		m_fmt = DXGI_FORMAT_UNKNOWN;
		m_hwnd = nullptr;
	}

	int BackbufferRenderTarget::Resize(int newWidth, int newHeiht, ID3D11Device* device)
	{
		if (newWidth <= 0 || newHeiht <= 0 || device==nullptr)
			return -1;
		if (m_swapchain == nullptr || m_hwnd==nullptr)
			return -2;
		ReleaseCOM(m_bkbufDxgiSurface);
		ReleaseCOM(m_renderTargetView);
		ReleaseCOM(m_depthView);
		ReleaseCOM(m_depthBuffer);
		ReleaseCOM(m_backbuffer);

		ID3D11Texture2D* backBuffer = NULL;
		D3D11_TEXTURE2D_DESC textureDesc;
		// Resize the swap chain and recreate the render target view.
		DXGI_SWAP_CHAIN_DESC sd_desc;
		m_swapchain->GetDesc(&sd_desc);
		HRESULT rslt = m_swapchain->ResizeBuffers(sd_desc.BufferCount, newWidth, newHeiht, sd_desc.BufferDesc.Format, sd_desc.Flags);
		if (FAILED(rslt))
		{
			return -3;
		}
		if (FAILED(m_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer))))
		{
			return -4;
		}
		if (NULL == backBuffer)
		{
			return -5;
		}
		backBuffer->GetDesc(&textureDesc);
		// Setup the description of the render target view.
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;
		if(FAILED(device->CreateRenderTargetView(backBuffer, &renderTargetViewDesc, &m_renderTargetView)))
		{
			ReleaseCOM(backBuffer);
			return -5;
		}

		if (m_bEnable4xMsaa && CheckMultisampleQualityLevels(textureDesc.Format, device)<=0)
		{
			ReleaseCOM(backBuffer);
			return -6;
		}
		if (0 != CreateDepthBufferAndView(newWidth, newHeiht, device))
		{
			ReleaseCOM(backBuffer);
			return -7;
		}
		m_backbuffer = backBuffer;
		return 0;

	}

	bool BackbufferRenderTarget::CheckMultisampleQualityLevels(DXGI_FORMAT fmt, ID3D11Device* device)
	{
		// Check 4X MSAA quality support for our back buffer format.
		// All Direct3D 11 capable devices support 4X MSAA for all render 
		// target formats, so we only need to check quality support.
		//UINT i4xMsaaQuality = 0;
		if (S_OK != device->CheckMultisampleQualityLevels(fmt, 4, &m_i4xMsaaQuality))
		{
			TCHAR errmsg[512] = { 0 };
			swprintf_s(errmsg, 512, L"Error in CheckMultisampleQualityLevels : CheckMultisampleQualityLevels failed.");
			log_e(LOG_TAG, errmsg);
		}
		if (m_i4xMsaaQuality <= 0)
		{
			TCHAR errmsg[512] = { 0 };
			swprintf_s(errmsg, 512, L"Error in CheckMultisampleQualityLevels : Need 4X MSAA, But the device is Not Support.");
			log_e(LOG_TAG, errmsg);
			return false;
		}
		return true;
	}

	bool BackbufferRenderTarget::CreateDepthBufferAndView(int width, int height, ID3D11Device* device)
	{
		// Create the depth/stencil buffer and view.
		D3D11_TEXTURE2D_DESC depthStencilDesc;
		depthStencilDesc.Width = width;
		depthStencilDesc.Height = height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

		// Use 4X MSAA? --must match swap chain MSAA values.
		if (m_bEnable4xMsaa)
		{
			depthStencilDesc.SampleDesc.Count = 4;
			depthStencilDesc.SampleDesc.Quality = m_i4xMsaaQuality - 1;
		}
		// No MSAA
		else
		{
			depthStencilDesc.SampleDesc.Count = 1;
			depthStencilDesc.SampleDesc.Quality = 0;
		}
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		ID3D11Texture2D* depthBuffer = nullptr;
		if (S_OK != device->CreateTexture2D(&depthStencilDesc, 0, &depthBuffer))
		{
			TCHAR errmsg[512] = { 0 };
			swprintf_s(errmsg, 512, L"Error in BackbufferRT : create depth buffer failed.");
			log_e(LOG_TAG, errmsg);
			return -8;
		}
		ID3D11DepthStencilView* depthView = nullptr;
		if (S_OK != device->CreateDepthStencilView(depthBuffer, 0, &depthView))
		{
 			TCHAR errmsg[512] = { 0 };
 			swprintf_s(errmsg, 512, L"Error in BackbufferRT : create depth stencil view failed.");
 			log_e(LOG_TAG, errmsg);
			ReleaseCOM(depthBuffer);
			return -9;
		}
		m_depthView = depthView;
		m_depthBuffer = depthBuffer;
		return 0;
	}

	void BackbufferRenderTarget::SetRenderTarget(ID3D11DeviceContext * ctx)
	{
		// Bind the render target view and depth/stencil view to the pipeline.
		if (ctx == nullptr)
			return;
		ctx->OMSetRenderTargets(1, &m_renderTargetView, m_depthView);
	}

	void BackbufferRenderTarget::ClearRenderTarget(ID3D11DeviceContext * ctx, float red, float green, float blue, float alpha)
	{
		if (nullptr == ctx)
			return;
		float color[4];
		// Setup the color to clear the buffer to.
		color[0] = red;
		color[1] = green;
		color[2] = blue;
		color[3] = alpha;

		// Clear the back buffer.
		ctx->ClearRenderTargetView(m_renderTargetView, color);
		// Clear the depth buffer.
		ctx->ClearDepthStencilView(/*depthStencilView*/m_depthView, D3D11_CLEAR_DEPTH, 1.0f, 0);

		return;
	}

	int BackbufferRenderTarget::LockBackbufferHDC(BOOL Discard, HDC* outHDC)
	{
		if (m_backbuffer == NULL)
		{
			log_e(L"BackbufferRenderTarget", L"DxRender_D3D11 Object have not be inited yet.\n");
			return DXRENDER_RESULT_NOT_INIT_YET;
		}
		if (m_bkbufDxgiSurface != NULL)
		{
			log_e(L"BackbufferRenderTarget", L"DxRender_D3D11 lock already and do not unlock.\n");
			return DXRENDER_RESULT_FUNC_REENTRY_INVALID;
		}

		HRESULT hr = m_swapchain->GetBuffer(0, __uuidof(IDXGISurface1), (void**)&m_bkbufDxgiSurface);

		if (SUCCEEDED(hr))
		{
			hr = m_bkbufDxgiSurface->GetDC(Discard, outHDC);
			if (SUCCEEDED(hr))
			{
				return DXRENDER_RESULT_OK;
			}
			else
			{
				ReleaseCOM(m_bkbufDxgiSurface);
			}
		}
		return DXRENDER_RESULT_FAILED;
	}

	int BackbufferRenderTarget::unlockBackbufferHDC(HDC hdc)
	{
		if (m_backbuffer == NULL)
		{
			log_e(LOG_TAG, _T("DxRender_D3D11 Object have not be inited yet.\n"));
			return DXRENDER_RESULT_NOT_INIT_YET;
		}
		if (m_bkbufDxgiSurface)
		{
			m_bkbufDxgiSurface->ReleaseDC(NULL);
			ReleaseCOM(m_bkbufDxgiSurface);
		}
		return DXRENDER_RESULT_OK;
	}

}//namespace zRender