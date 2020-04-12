#include "DxRender_D3D11.h"
#include "d3dAdapterOutputEnumerator.h"
#include <assert.h>
#include <fstream>
#include "YUVTexture_Packed.h"
#include "YUVTexture_Planar.h"
#include "ARGBTexture_8.h"
#include "YUVTexture_NV12.h"
#include <DxErr.h>
#include "DXLogger.h"
#include "libtext.h"
#include "inc/TextureResource.h"
#include "D3D11TextureRender.h"
#include "BackbufferRT.h"
#include "ConstDefine.h"

using namespace zRender;

#define LOG_TAG L"DxRender_D3D11"

int createDeviceAndContext(IDXGIAdapter* adapter, UINT createDeviceFlags,
	ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext, D3D_FEATURE_LEVEL& level);

DxRender_D3D11::DxRender_D3D11()
	: m_adapter(NULL), m_device(NULL), m_context(NULL)
	, m_hWnd(NULL), m_aspectRatio(0)
	, m_bkbufDxgiSurface(NULL)
	, m_TransparentBS(NULL)
	, m_renderTargetView(NULL)
	, m_renderTargetTexture(NULL), m_bkbufRT(nullptr)
	, m_color(0)
	, m_offscreenRttRender(NULL)
{
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_viewTransform, I);
	XMStoreFloat4x4(&m_projTransform, I);
}

DxRender_D3D11::~DxRender_D3D11()
{
//	cancleBackground();
	deinit();
}

int DxRender_D3D11::init(HWND hWnd, bool isEnable4XMSAA /*= false*/, bool isSDICompatible/* = false*/)
{
	if (nullptr != m_bkbufRT)
	{
		log_e(LOG_TAG, L"Backbuffer render target have inited before.");
		return -1;
	}

	IDXGIAdapter* dstAdapter = NULL;
	if(S_OK!=DXGI_getNearestAdapter(hWnd, &dstAdapter))
	{
		log_e(LOG_TAG, L"DXGI get adapter from hWnd failed.");
		return -2;
	}

	DXGI_ADAPTER_DESC adptDesc;
	if(S_OK==dstAdapter->GetDesc(&adptDesc))
	{
		log_e(LOG_TAG, libtext::format(L"DXGI get adapter from hWnd success.Adapter name [%s]", adptDesc.Description));
	}
	UINT createDeviceFlags = 0;
//#if defined(DEBUG) || defined(_DEBUG)  
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
//#endif
	D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_9_1;
	int ret = createDeviceAndContext(dstAdapter, createDeviceFlags, &m_device, &m_context, level);
	if (0 != ret)
	{
		TCHAR errmsg[512] = { 0 };
		swprintf_s(errmsg, 512, L"Error in createDeviceAndContext : ret=%d", ret);
		log_e(LOG_TAG, errmsg);
		return -4;
	}
	dstAdapter->Release();

	BackbufferRenderTarget* bkbufRT = new BackbufferRenderTarget();
	ret = bkbufRT->Initialize(m_device, hWnd, DXGI_FORMAT_B8G8R8A8_UNORM, 
		isEnable4XMSAA, isSDICompatible);
	if (0 != ret)
	{
		delete bkbufRT;
		TCHAR errmsg[512] = { 0 };
		swprintf_s(errmsg, 512, L"DxRender_D3D11::init: create and init backbuffer failed.hWnd=%u isEnable4XMSAA=%d isSDICompatible=%d",
			(UINT)hWnd, isEnable4XMSAA, isSDICompatible);
		log_e(LOG_TAG, errmsg);
		return -6;
	}
	m_bkbufRT = bkbufRT;
	m_hWnd = hWnd;
	m_bkbufRT->SetRenderTarget(m_context);

	// Set the viewport transform.
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width    = static_cast<float>(m_bkbufRT->GetWidth());
	m_viewport.Height   = static_cast<float>(m_bkbufRT->GetHeight());
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_context->RSSetViewports(1, &m_viewport);

// 	m_material.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
// 	m_material.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
// 	m_material.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);

	CreateTransparentBlendState();
	log_e(LOG_TAG, L"init success.");
	return 0;
}

bool zRender::DxRender_D3D11::CreateTransparentBlendState()
{
	//create transparent blend state
	D3D11_BLEND_DESC transparentDesc = { 0 };
	transparentDesc.AlphaToCoverageEnable = false;
	transparentDesc.IndependentBlendEnable = false;
	transparentDesc.RenderTarget[0].BlendEnable = true;
	transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	if (FAILED(m_device->CreateBlendState(&transparentDesc, &m_TransparentBS)))
	{
		TCHAR errmsg[512] = { 0 };
		swprintf_s(errmsg, 512, L"Error in DxRender_D3D11::init : create alpha blend state failed.");
		log_e(LOG_TAG, errmsg);
		return false;
	}
	return true;
}

int zRender::DxRender_D3D11::init(HMONITOR hmonitor)
{
	IDXGIAdapter* dstAdapter = NULL;
	if(S_OK!=DXGI_getAdapter(hmonitor, &dstAdapter))
	{
		log_e(LOG_TAG, L"DXGI get adapter from hWnd failed.");
		return -1;
	}

	DXGI_ADAPTER_DESC adptDesc;
	if(S_OK==dstAdapter->GetDesc(&adptDesc))
	{
		log_e(LOG_TAG, libtext::format(L"DXGI get adapter from hWnd success.Adapter name [%s]", adptDesc.Description));
	}
	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL curFeatureLv = D3D_FEATURE_LEVEL_9_1;
	D3D_FEATURE_LEVEL featureLvs[] = {
		//D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	HRESULT hr = D3D11CreateDevice(
		dstAdapter,                 // use selected adapter
		D3D_DRIVER_TYPE_UNKNOWN,	//adapter is No NULL
		0,							// no software device
		createDeviceFlags, 
		featureLvs, ARRAYSIZE(featureLvs),// default feature level array
		D3D11_SDK_VERSION,
		&m_device,
		&curFeatureLv,
		&m_context);
	if(S_OK!=hr)
	{
		TCHAR errmsg[512] = {0};
		swprintf_s(errmsg, 512, L"Error in DxRender_D3D11::init : faile to Create device with param HMONITOR(%u) Adapter(%d) CreateFlag(%d) ErrorCode=%d",
			(unsigned int)hmonitor, (int)dstAdapter, createDeviceFlags, (int)hr);
#ifdef _DEBUG
		printf("Error in DxRender_D3D11::init : faile to Create device with param Adapter(%d) CreateFlag(%d) ErrorCode=%d\n",
			(int)dstAdapter, createDeviceFlags, (int)hr);
#endif
		log_e(LOG_TAG, errmsg);
		return -2;
	}
	dstAdapter->Release();

	if( curFeatureLv < D3D_FEATURE_LEVEL_11_0 )
	{
#ifdef _DEBUG
		printf("Error in DxRender_D3D11::init : Direct3D Feature Level 11 unsupported.\n");
#endif
		TCHAR errmsg[512] = {0};
		swprintf_s(errmsg, 512, L"Error in DxRender_D3D11::init : Direct3D Feature Level 11 unsupported. FeatureLevel=%d",
			(int)curFeatureLv);
		log_e(LOG_TAG, errmsg);
		return -3;
	}

	return 0;
}

int createDeviceAndContext(IDXGIAdapter* adapter, UINT createDeviceFlags, 
	ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext, D3D_FEATURE_LEVEL& level)
{
	if (ppDevice == nullptr || ppContext == nullptr)
		return -1;
	D3D_FEATURE_LEVEL curFeatureLv = D3D_FEATURE_LEVEL_9_1;
	D3D_FEATURE_LEVEL featureLvs[] = {
		//D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* context = nullptr;
	HRESULT hr = D3D11CreateDevice(
		adapter,			         // use selected adapter
		D3D_DRIVER_TYPE_UNKNOWN,	//adapter is No NULL
		0,							// no software device
		createDeviceFlags,
		featureLvs, ARRAYSIZE(featureLvs),// default feature level array
		D3D11_SDK_VERSION,
		&device,
		&curFeatureLv,
		&context);
	if (S_OK != hr)
	{
		TCHAR errmsg[512] = { 0 };
		swprintf_s(errmsg, 512, L"Error in DxRender_D3D11::init : faile to Create device with param Adapter(%d) CreateFlag(%d) ErrorCode=%d",
			(int)adapter, createDeviceFlags, (int)hr);
#ifdef _DEBUG
		printf("Error in DxRender_D3D11::init : faile to Create device with param Adapter(%d) CreateFlag(%d) ErrorCode=%d\n",
			(int)dstAdapter, createDeviceFlags, (int)hr);
#endif
		log_e(LOG_TAG, errmsg);
		return -4;
	}

	if (curFeatureLv < D3D_FEATURE_LEVEL_11_0)
	{
#ifdef _DEBUG
		printf("Error in DxRender_D3D11::init : Direct3D Feature Level 11 unsupported.\n");
#endif
		TCHAR errmsg[512] = { 0 };
		swprintf_s(errmsg, 512, L"Error in DxRender_D3D11::init : Direct3D Feature Level 11 unsupported. FeatureLevel=%d",
			(int)curFeatureLv);
		log_e(LOG_TAG, errmsg);
		return -5;
	}
	*ppDevice = device;
	*ppContext = context;
	level = curFeatureLv;
	return 0;
}

int checkMultiSampleLevel(DXGI_FORMAT fmt, int sampleCount)
{
	// fixme
	return -1;
}

int zRender::DxRender_D3D11::init( int width, int height, int adapter/*=0*/, bool isSDICompatible /*= false*/ )
{
	if (isRenderTargetSetted())
	{
		//不可调用多次，除非先Deinit
		return DXRENDER_RESULT_FUNC_REENTRY_INVALID;
	}
	//判断参数是否有效
	std::vector<IDXGIAdapter*> adapterVec;
	if(S_OK!=DXGI_getAdapters(adapterVec))
	{
		log_e(LOG_TAG, L"DXGI get adapter list failed.");
		return DXRENDER_RESULT_INTERNAL_ERR;
	}
	if(adapter>=(int)adapterVec.size())
	{
		log_e(LOG_TAG, L"Param invalid. adapter is too big");
		DXGI_releaseAdaptersObjs(adapterVec);
		return DXRENDER_RESULT_PARAM_INVALID;
	}

	if(width<=0 || height<=0)
	{
		log_e(LOG_TAG, L"Param invalid. width height");
		DXGI_releaseAdaptersObjs(adapterVec);
		return DXRENDER_RESULT_PARAM_INVALID;
	}
	UINT createDeviceFlags = 0;
	//#if defined(DEBUG) || defined(_DEBUG)  
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	//#endif
	D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_9_1;
	int ret = createDeviceAndContext(adapterVec[adapter], createDeviceFlags, &m_device, &m_context, level);
	DXGI_releaseAdaptersObjs(adapterVec);
	if (0 != ret)
	{
		TCHAR errmsg[512] = { 0 };
		swprintf_s(errmsg, 512, L"Error in createDeviceAndContext : ret=%d", ret);
		log_e(LOG_TAG, errmsg);
		return DXRENDER_RESULT_FAILED;
	}
/*	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.
	UINT i4xMsaaQuality = 0;
	if(S_OK!=m_device->CheckMultisampleQualityLevels(DXGI_FORMAT_B8G8R8A8_UNORM, 4, &i4xMsaaQuality))
	{
#ifdef _DEBUG
		printf("Error in DxRender_D3D11::init : CheckMultisampleQualityLevels failed.\n");
#endif
		TCHAR errmsg[512] = {0};
		swprintf_s(errmsg, 512, L"Error in DxRender_D3D11::init : CheckMultisampleQualityLevels failed.");
		log_e(LOG_TAG, errmsg);
	}
	if( i4xMsaaQuality<=0 && isEnable4XMSAA)
	{
#ifdef _DEBUG
		printf("Error in DxRender_D3D11::init : check quality support failed\n");
#endif
		TCHAR errmsg[512] = {0};
		swprintf_s(errmsg, 512, L"Error in DxRender_D3D11::init : Need 4X MSAA, But the device is Not Support.");
		log_e(LOG_TAG, errmsg);
		return -6;
	}*/

	m_renderTargetTexture = new RenderTextureClass();
	if (!m_renderTargetTexture->Initialize(m_device, width, height, DXGI_FORMAT_B8G8R8A8_UNORM))
	{
		return DXRENDER_RESULT_FAILED;
	}
	if (0 != (ret=setRenderTargetTexture()))
	{
		TCHAR errmsg[1024] = { 0 };
		swprintf_s(errmsg, 1024, L"Error in DxRender_D3D11::init : setRenderTargetTexture failed. ret=%d", ret);
		log_e(LOG_TAG, errmsg);
		return DXRENDER_RESULT_FAILED;
	}
	CreateTransparentBlendState();
	//	float blendFactors[] = {0.0f, 0.0f, 0.0f, 0.0f};
	//	m_context->OMSetBlendState(	m_TransparentBS, blendFactors, 0xffffffff);
	log_e(LOG_TAG, L"init success.");
	return 0;
}

void DxRender_D3D11::deinit()
{
	releaseOffscreenRenderTarget();
	if (m_bkbufRT)
	{
		delete m_bkbufRT;
		m_bkbufRT = nullptr;
	}
	ReleaseCOM(m_TransparentBS);
	ReleaseCOM(m_renderTargetView);
	// Restore all default settings.
	if( m_context )
		m_context->ClearState();
	ReleaseCOM(m_context);
	ReleaseCOM(m_device);
}

int DxRender_D3D11::setVisibleRegion(const RECT_f& visibleReg)
{
	if(visibleReg.width()<=0 || visibleReg.height()<=0
		|| visibleReg.left<0 || visibleReg.top<0)
		return DXRENDER_RESULT_PARAM_INVALID;
	if (!isRenderTargetSetted())
		return DXRENDER_RESULT_NOT_INIT_YET;

	//根据窗口的宽高比设置从物体坐标系到world坐标系的基本转换矩阵
	float aspectRatio = this->getWidth() / (float)this->getHeight();

	//设置生成从world坐标系到人眼坐标系的转换矩阵
	//XMVECTOR pos = XMVectorSet((visibleReg.left+visibleReg.right)*aspectRatio / 2, (visibleReg.top+visibleReg.bottom) / 2, -5, 1.0f);
	XMVECTOR pos = XMVectorSet(visibleReg.left*aspectRatio+visibleReg.width()*aspectRatio/2, -1.0f*(visibleReg.top+visibleReg.bottom) / 2, POS_ZINDEX_OF_EYE, 1.0f);
	//XMVECTOR target = XMVectorSet((visibleReg.left+visibleReg.right)*aspectRatio / 2, (visibleReg.top+visibleReg.bottom) / 2, 0, 1.0f);//XMVectorZero();
	XMVECTOR target = XMVectorSet(visibleReg.left*aspectRatio+visibleReg.width()*aspectRatio/2, -1.0f*(visibleReg.top+visibleReg.bottom) / 2, 0, 1.0f);//XMVectorZero();
	XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_viewTransform, V);

	//生成Projection的转换矩阵
	XMMATRIX P = XMMatrixOrthographicLH(visibleReg.width()*aspectRatio, visibleReg.height(), RANGE_OF_ZINDEX_MIN, RANGE_OF_ZINDEX_MAX);
	//XMMATRIX P = XMMatrixOrthographicLH(1, 1, 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_projTransform, P);

	m_visibleReg = visibleReg;
	m_aspectRatio = aspectRatio;
	return 0;
}

RECT_f DxRender_D3D11::getVisibleRegion() const
{
	return m_visibleReg;
}

// fixme DxRender* dxRender 是不需要的，此处是临时为通过编译而这样的
DisplayElement* DxRender_D3D11::createDisplayElement(const RECT_f& displayReg, int zIndex, DxRender* dxRender)
{
	if(m_device==NULL)
	{
		log_e(LOG_TAG, _T("DxRender_D3D11 Object have not be inited yet.\n"));
		return NULL;
	}

	if(!displayReg.isIntersect(m_visibleReg))
	{
		TCHAR errmsg[1024] = {0};
		swprintf_s(errmsg, 1024, L"Failed To Create DisplayElement[L-%f, T-%f, R-%f, B-%f], out of DxRender Visible region[L-%f, T-%f, R-%f, B-%f].\n",
				displayReg.left, displayReg.top, displayReg.right, displayReg.bottom,
				m_visibleReg.left, m_visibleReg.top, m_visibleReg.right, m_visibleReg.bottom);
		log_e(LOG_TAG, errmsg);
		return NULL;
	}

	DisplayElement* de = new DisplayElement(dxRender, m_device, m_context);
	assert(de);
	int ret = de->setDisplayRegion(displayReg, (float)zIndex);
	assert(0==ret);
	if(0!=ret)
	{
		TCHAR errmsg[512] = {0};
		swprintf_s(errmsg, 512, L"Failed to setDisplayRegion of display elem.");
		log_e(LOG_TAG, errmsg);
	}
	return de;
}

int DxRender_D3D11::releaseDisplayElement(DisplayElement** displayElement)
{
	if(NULL==displayElement)
		return -1;
	if(*displayElement!=NULL)
	{
		DisplayElement* pelem = *displayElement;
		int ret = pelem->releaseRenderResource();
		if(0!=ret)
			return ret;
		//delete *displayElement;
		delete pelem;
		*displayElement = NULL;
	}
	return 0;
}

IRawFrameTexture* DxRender_D3D11::createTexture(PIXFormat pixFmt, int width, int height, 
					unsigned char* initData/* = NULL*/, int initDataLen /*= 0*/, bool isShared /*= false*/)
{
	if(m_device==NULL)
	{
		log_e(LOG_TAG, _T("DxRender_D3D11 Object have not be inited yet.\n"));
		return NULL;
	}

	if(pixFmt==PIXFMT_UNKNOW || width<=0 || height<=0 
		|| (initData!=NULL && initDataLen<=0) || (initData==NULL &&initDataLen>0))
	{
		TCHAR errmsg[1024] = {0};
		swprintf_s(errmsg, 1024, L"DxRender_D3D11 failed to create Texture.Param invalid.[Fmt=%d W=%d H=%d data=%d dataLen=%d]",
				pixFmt, width, height, (int)initData, initDataLen);
		log_e(LOG_TAG, errmsg);
		return NULL;
	}

	//YUVTexture_Packed* tex = new YUVTexture_Packed(pixFmt);
	IRawFrameTexture* tex = NULL;
	switch(pixFmt)
	{
	case PIXFMT_A8R8G8B8:
	case PIXFMT_R8G8B8:
	case PIXFMT_R8G8B8A8:
	case PIXFMT_B8G8R8A8:
	case PIXFMT_B8G8R8X8:
	case PIXFMT_X8R8G8B8:
		tex = new ARGBTexture_8(pixFmt);
		break;
	case PIXFMT_YUY2:
		tex = new YUVTexture_Packed(pixFmt);
		break;
	case PIXFMT_YUV420P:
	case PIXFMT_YV12:
		tex = new YUVTexture_Planar(pixFmt);
		break;
	case PIXFMT_NV12:
		tex = new YUVTexture_NV12(pixFmt);
		break;
	default:
		{
			TCHAR errmsg[1024] = {0};
			swprintf_s(errmsg, 1024, L"DxRender_D3D11 failed to create Texture.unsurport Pixfmt.[Fmt=%d]", pixFmt);
			log_e(LOG_TAG, errmsg);
		}
		break;
	}
	if(NULL==tex || 0!=tex->create(m_device, width, height, (char*)initData, initDataLen))
	{
		delete tex;
		TCHAR errmsg[1024] = {0};
		swprintf_s(errmsg, 1024, L"tex->create failed");
		log_e(LOG_TAG, errmsg);
		return NULL;
	}

	return tex;
}

IRawFrameTexture * DxRender_D3D11::createTexture(PIXFormat pixfmt, int width, int height, TEXTURE_USAGE usage, bool bShared, unsigned char * initData, int dataLen, int pitch)
{
	if (m_device == NULL)
	{
#ifdef _DEBUG
		printf("DxRender_D3D11 Object have not be inited yet.\n");
		assert(false);
#endif
		return NULL;
	}
	IRawFrameTexture* tex = NULL;
	switch (pixfmt)
	{
	case PIXFMT_A8R8G8B8:
	case PIXFMT_R8G8B8:
	case PIXFMT_R8G8B8A8:
	case PIXFMT_B8G8R8A8:
	case PIXFMT_B8G8R8X8:
	case PIXFMT_X8R8G8B8:
		tex = new ARGBTexture_8(pixfmt);
		break;
	case PIXFMT_YUY2:
		tex = new YUVTexture_Packed(pixfmt);
		break;
	case PIXFMT_YUV420P:
	case PIXFMT_YV12:
		tex = new YUVTexture_Planar(pixfmt);
		break;
	case PIXFMT_NV12:
		tex = new YUVTexture_NV12(pixfmt);
		break;
	default:
	{
		TCHAR errmsg[1024] = { 0 };
		swprintf_s(errmsg, 1024, L"DxRender_D3D11 failed to create Texture.unsurport Pixfmt.[Fmt=%d]", pixfmt);
		log_e(LOG_TAG, errmsg);
	}
	break;
	}
	if (NULL == tex || 0 != tex->create(m_device, width, height, usage, bShared, (char*)initData, dataLen, pitch))
	{
		delete tex;
		TCHAR errmsg[1024] = { 0 };
		swprintf_s(errmsg, 1024, L"tex->create failed");
		log_e(LOG_TAG, errmsg);
		return NULL;
	}

	return tex;
}

IRawFrameTexture * zRender::DxRender_D3D11::openSharedTexture(IRawFrameTexture * sharedTexture)
{
	if (sharedTexture == NULL)
		return NULL;
	PIXFormat pixfmt = sharedTexture->getPixelFormat();
	int width = sharedTexture->getWidth();
	int height = sharedTexture->getHeight();
	IRawFrameTexture* tex = NULL;
	switch (pixfmt)
	{
	case PIXFMT_A8R8G8B8:
	case PIXFMT_R8G8B8:
	case PIXFMT_R8G8B8A8:
	case PIXFMT_B8G8R8A8:
	case PIXFMT_B8G8R8X8:
	case PIXFMT_X8R8G8B8:
		tex = new ARGBTexture_8(pixfmt);
		break;
	case PIXFMT_YUY2:
		tex = new YUVTexture_Packed(pixfmt);
		break;
	case PIXFMT_YUV420P:
	case PIXFMT_YV12:
		tex = new YUVTexture_Planar(pixfmt);
		break;
	case PIXFMT_NV12:
		tex = new YUVTexture_NV12(pixfmt);
		break;
	default:
	{
		TCHAR errmsg[1024] = { 0 };
		swprintf_s(errmsg, 1024, L"DxRender_D3D11 failed to create Texture.unsurport Pixfmt.[Fmt=%d]", pixfmt);
		log_e(LOG_TAG, errmsg);
	}
	break;
	}
	if (NULL == tex || 0 != tex->openSharedTexture(m_device, sharedTexture))
	{
		delete tex;
		TCHAR errmsg[1024] = { 0 };
		swprintf_s(errmsg, 1024, L"tex->openSharedTexture failed");
		log_e(LOG_TAG, errmsg);
		return NULL;
	}
	return tex;
}

int DxRender_D3D11::releaseTexture(IRawFrameTexture** texture)
{
	if(NULL==texture)
		return -1;
	if(*texture!=NULL)
	{
		int ret = (*texture)->destroy();
		if(0!=ret)
			return ret;
		delete *texture;
		*texture = NULL;
	}
	return 0;
}

ID3D11Buffer* DxRender_D3D11::createBuffer(int byteCount, const unsigned char* initData, int initDataLen,
									D3D11_USAGE usage, UINT bindFlag)
{
	if(m_device==NULL)
	{
		log_e(LOG_TAG, _T("DxRender_D3D11 Object have not be inited yet.\n"));
		return NULL;
	}
	if(byteCount<16 || initData==NULL || initDataLen<byteCount)
	{
		TCHAR errmsg[1024] = { 0 };
		swprintf_s(errmsg, 1024, _T("Error in DxRender_D3D11::createVertexBuffer : param invalid.(BC=%d, data=%d, dataLen=%d)\n"),
			byteCount, (int)initData, initDataLen);
		log_e(LOG_TAG, errmsg);
		return NULL;
	}
	ID3D11Buffer* vtBuf = NULL;
	D3D11_BUFFER_DESC vbd;
    vbd.Usage = usage;
    vbd.ByteWidth = byteCount;
    vbd.BindFlags = bindFlag;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = initData;
    if(S_OK!=m_device->CreateBuffer(&vbd, &vinitData, &vtBuf))
	{
		return NULL;
	}
	return vtBuf;
}

ID3D11Buffer* DxRender_D3D11::createVertexBuffer(int byteCount, const unsigned char* initData, int initDataLen)
{
	return createBuffer(byteCount, initData, initDataLen, D3D11_USAGE_IMMUTABLE, D3D11_BIND_VERTEX_BUFFER);
}

ID3D11Buffer* DxRender_D3D11::createIndexBuffer(int byteCount, const unsigned char* initData, int initDataLen)
{
	return createBuffer(byteCount, initData, initDataLen, D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER);
}

int DxRender_D3D11::releaseBuffer(ID3D11Buffer** buffer)
{
	if(buffer==NULL)	return -1;
	if(*buffer)
	{
		(*buffer)->Release();
		*buffer = NULL;
	}
	return 0;
}

int DxRender_D3D11::draw(DisplayElement* displayElem)
{
	if (m_bkbufRT != nullptr)
	{
		setRenderTargetBackbuffer();
	}
	else if (m_renderTargetTexture)
	{
		setRenderTargetTexture();
	}
	if(m_device==NULL || !isRenderTargetSetted())
	{
#ifdef _DEBUG
		printf("DxRender_D3D11 Object have not be inited yet.\n");
		assert(false);
#endif
		return -1;
	}
	if(displayElem==NULL)
	{
#ifdef _DEBUG
		printf("Error in DxRender_D3D11::draw : param displayElem is NULL.\n");
#endif
		return -2;
	}
// 	if (displayElem->getParentDxRender() != this)
// 	{
// 		log_e(LOG_TAG, L"Error in DxRender_D3D11::draw : the display elemement is not created by this DxRender object.");
// 		return -3;
// 	}
	return displayElem->draw();
}

int DxRender_D3D11::present(int type)
{
	// 只有绘制到Backbuffer中时才需要调用SwapChain的Present
	// 如果绘制到Texture中，则无需调用该接口
	if(!isRenderTargetSetted())	
		return DXRENDER_RESULT_NOT_INIT_YET;
	if (m_bkbufRT != NULL && nullptr != m_bkbufRT->getSwapchain())
	{
		m_bkbufRT->getSwapchain()->Present(type, 0);
	}

// 	if (m_renderTargetTexture != NULL)
// 	{
// 		drawOffscreenRenderTarget();
// 	}
	
	//getSnapshot(NULL);
	return 0;
}

int DxRender_D3D11::clear(DWORD color)
{
	m_color = color;
	if (m_renderTargetTexture)
	{
		return clearRenderTargetTexture(color);
	}
	else
	{
		return clearBackbuffer(color);
	}
}

int zRender::DxRender_D3D11::clearBackbuffer(DWORD color)
{
	if (m_bkbufRT == nullptr)
		return -1;
	int a = (color & 0xff000000) >> 24;
	int r = (color & 0x00ff0000) >> 16;
	int g = (color & 0x0000ff00) >> 8;
	int b = (color & 0x000000ff) >> 0;
	float colorBack[4] = { (float)r / 256, (float)g / 256, (float)b / 256, (float)a / 256 };
	m_bkbufRT->ClearRenderTarget(m_context, (float)r / 256, (float)g / 256, (float)b / 256, (float)a / 256);
	//m_context->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const float*>(colorBack));
	//m_context->ClearDepthStencilView(m_depthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	return 0;
}


int zRender::DxRender_D3D11::clearRenderTargetTexture(DWORD color)
{
	if (m_renderTargetTexture == nullptr)
		return -1;
	int a = (color & 0xff000000) >> 24;
	int r = (color & 0x00ff0000) >> 16;
	int g = (color & 0x0000ff00) >> 8;
	int b = (color & 0x000000ff) >> 0;
	m_renderTargetTexture->ClearRenderTarget(m_context, (float)r / 256, (float)g / 256, (float)b / 256, (float)a / 256);
	return 0;
}

int zRender::DxRender_D3D11::lockBackbufferHDC( BOOL Discard, HDC* outHDC )
{
	if (m_bkbufRT != NULL)
	{
		return m_bkbufRT->LockBackbufferHDC(Discard, outHDC);
	}
	if (m_renderTargetTexture != NULL)
	{
		return m_renderTargetTexture->LockBackbufferHDC(Discard, outHDC);
	}
	return DXRENDER_RESULT_FAILED;
}

int zRender::DxRender_D3D11::unlockBackbufferHDC( HDC hdc )
{
	if (m_bkbufRT != NULL)
	{
		return m_bkbufRT->unlockBackbufferHDC(hdc);
	}
	if (m_renderTargetTexture != NULL)
	{
		return m_renderTargetTexture->unlockBackbufferHDC(hdc);
	}
	return DXRENDER_RESULT_FAILED;
}

void* zRender::DxRender_D3D11::getDevice() const
{
	return (void*)m_device;
}

int zRender::DxRender_D3D11::getWidth()
{
	if (!isRenderTargetSetted())
		return 0;
	return m_bkbufRT ? m_bkbufRT->GetWidth() : m_renderTargetTexture->GetWidth();
}

int zRender::DxRender_D3D11::getHeight()
{
	if (!isRenderTargetSetted())
		return 0;
	return m_bkbufRT ? m_bkbufRT->GetHeight() : m_renderTargetTexture->GetHeight();
}

#include "DxRender_D3D9.h"
#include <D3DX11tex.h>
int zRender::DxRender_D3D11::getSnapshot( unsigned char* pData, UINT& datalen, int& w, int& h, int& pixfmt, int& pitch )
{
	if (pData == NULL || datalen <= 0)
		return DXRENDER_RESULT_PARAM_INVALID;
	//这个方法的实现性能并不是很高，所以当DxRender_D3D11对象渲染的目标分辨率>=1208*720时，以25fps的帧率调用此接口可能导致CPU上升2~4%（I5）
	if(m_device==NULL || NULL==m_context  || !isRenderTargetSetted())
	{
		return DXRENDER_RESULT_NOT_INIT_YET;
	}
	ID3D11Texture2D* backBuffer = getRenderTargetTexture();
	if(NULL==backBuffer)
	{
		return DXRENDER_RESULT_NOT_INIT_YET;
	}
	
	D3D11_TEXTURE2D_DESC backbufDesc;
	backBuffer->GetDesc(&backbufDesc);
	//用BackBuffer的DESC作为基础创建STAGE模式的TEXTURE2D用于将BackBuffer的内容拷贝出来
	backbufDesc.MipLevels = backbufDesc.ArraySize = 1;
	backbufDesc.SampleDesc.Count = 1;
	backbufDesc.Usage = D3D11_USAGE_STAGING;		//staging mode is more faster
	backbufDesc.BindFlags = 0;
	backbufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	backbufDesc.MiscFlags = 0;
	ID3D11Texture2D* outputTexture = NULL;
	HRESULT hr = m_device->CreateTexture2D(&backbufDesc, NULL, &outputTexture);
	if(FAILED(hr))
	{
		return DXRENDER_RESULT_INTERNAL_ERR;
	}
	m_context->CopyResource(outputTexture, backBuffer);
	//hr = D3DX11SaveTextureToFile(m_context, outputTexture, D3DX11_IFF_JPG, L"11tex.jpg");

	D3D11_MAPPED_SUBRESOURCE resource;
	unsigned int subresource = D3D11CalcSubresource(0, 0, 0);
	hr = m_context->Map(outputTexture, subresource, D3D11_MAP_READ/*D3D11_MAP_READ_WRITE*/, 0, &resource);
	//resource.pData; // TEXTURE DATA IS HERE
	if (FAILED(hr))
	{
		ReleaseCOM(outputTexture);
		return DXRENDER_RESULT_INTERNAL_ERR;
	}
	//将数据从Texture中拷贝到调用者提供的内存中，并设置相关参数
	D3D11_TEXTURE2D_DESC outbufDesc;
	outputTexture->GetDesc(&outbufDesc);
	const int srcpitch = resource.RowPitch;
	const unsigned char* source = static_cast< const unsigned char* >(resource.pData);

	if(srcpitch*outbufDesc.Height > datalen)
	{
		m_context->Unmap(outputTexture, subresource);
		ReleaseCOM(outputTexture);
		return DXRENDER_RESULT_PARAM_INVALID;
	}

// 		static unsigned int framecount = 0;
//  	char strFileName[128] = { 0 };
//  	sprintf(strFileName, "outfile2_%d_%d_%u.rgb", outbufDesc.Width, outbufDesc.Height, framecount);
//  	std::ofstream* imgfile = new std::ofstream(strFileName, std::ios::out | std::ios::binary);
//  	for (size_t i = 0; i < outbufDesc.Height; ++i)
//  	{
//  		const unsigned char* psrc = source + i*srcpitch;
//  		imgfile->write((const char*)psrc, srcpitch);
//  	}
//  	delete imgfile;
//	 	framecount++;
 	for (size_t i = 0; i < outbufDesc.Height; ++i)
 	{
 		const unsigned char* psrc = source + i*srcpitch;
 		unsigned char* pdst = pData + i*srcpitch;
		memcpy(pdst, psrc, srcpitch);
 	}
	w = outbufDesc.Width;
	h = outbufDesc.Height;
	pitch = srcpitch;
	datalen = outbufDesc.Height * srcpitch;
	pixfmt = (int)outbufDesc.Format;
	//释放相关对象
	m_context->Unmap(outputTexture, subresource);
	ReleaseCOM(outputTexture);
	return 0;
}

TextureResource * zRender::DxRender_D3D11::getSnapshot(TEXTURE_USAGE usage, bool bShared, bool fromOffscreenTexture)
{
	if (TEXTURE_USAGE_STAGE == usage && bShared) return NULL;
	ID3D11Texture2D* renderTargetTex = getRenderTargetTexture();
	if (NULL == renderTargetTex)	return NULL;
	D3D11_TEXTURE2D_DESC desc;
	renderTargetTex->GetDesc(&desc);
	TextureResource* frameTexture = new TextureResource();
	if (0 != frameTexture->create(m_device, desc.Width, desc.Height, desc.Format, usage, bShared, NULL, 0, 0))
	{
		delete frameTexture;
		return NULL;
	}
	if (0 != frameTexture->copyTexture(renderTargetTex))
	{
		delete frameTexture;
	}
	if (NULL == m_renderTargetTexture)
	{
		// 此时renderTargetTex为从SwapChain中获取的Texture（backbuffer）
		ReleaseCOM(renderTargetTex);
	}
	return frameTexture;
}

int zRender::DxRender_D3D11::createOffscreenRenderTarget(int width, int height)
{
	if (m_device == NULL)
		return -1;
	if (width <= 0 || height <= 0 || width > 1920 * 4 || height > 1080 * 4)
		return -2;
	if (m_renderTargetTexture)	return -3;
	m_renderTargetTexture = new RenderTextureClass();
	if (!m_renderTargetTexture->Initialize(m_device, width, height, DXGI_FORMAT_B8G8R8A8_UNORM))
	{
		return -4;
	}
	if (m_offscreenRttRender == NULL)
	{
		m_offscreenRttRender = new D3D11TextureRender(m_device, m_context);
	}
	return 0;
}

void zRender::DxRender_D3D11::releaseOffscreenRenderTarget()
{
	if (m_offscreenRttRender)
	{
		delete m_offscreenRttRender;
		m_offscreenRttRender = NULL;
	}
	if (m_renderTargetTexture)
	{
		m_renderTargetTexture->Shutdown();
		delete m_renderTargetTexture;
		m_renderTargetTexture = NULL;
	}
}

int zRender::DxRender_D3D11::setRenderTargetTexture()
{
	if (NULL == m_renderTargetTexture)
		return -1;
	m_renderTargetTexture->SetRenderTarget(m_context);
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = static_cast<float>(m_renderTargetTexture->GetWidth());
	m_viewport.Height = static_cast<float>(m_renderTargetTexture->GetHeight());
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_context->RSSetViewports(1, &m_viewport);
	return 0;
}

int zRender::DxRender_D3D11::setRenderTargetBackbuffer()
{
	//if (m_swapChain == NULL || m_renderTargetView==NULL)
	if(nullptr==m_bkbufRT)
		return -1;
	m_bkbufRT->SetRenderTarget(m_context);
	//m_context->OMSetRenderTargets(1, &m_renderTargetView, m_depthView);
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = static_cast<float>(m_bkbufRT->GetWidth());
	m_viewport.Height = static_cast<float>(m_bkbufRT->GetHeight());
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_context->RSSetViewports(1, &m_viewport);
	return 0;
}

int zRender::DxRender_D3D11::drawOffscreenRenderTarget()
{
	if (NULL == m_renderTargetTexture)
		return -1;
	if (m_offscreenRttRender == NULL)
		return -2;
	RECT rect;
	rect.left = rect.top = 0;
	rect.right = getWidth();
	rect.bottom = getHeight();
	//return m_offscreenRttRender->draw(m_renderTargetTexture->getRenderTargetTexture(), m_renderTargetTexture->GetShaderResourceView(), rect, m_swapChain); //fixme 这个调用可能崩溃
	return m_offscreenRttRender->draw(m_renderTargetTexture->getRenderTargetTexture(), m_renderTargetTexture->GetShaderResourceView(), rect, 1, &m_renderTargetView);
}

ID3D11Texture2D * zRender::DxRender_D3D11::getRenderTargetTexture()
{
	if (m_renderTargetTexture)
	{
		return m_renderTargetTexture->getRenderTargetTexture();
	}
	else if(m_bkbufRT!=nullptr)
	{
		return m_bkbufRT->getRenderTargetTexture();
/*		if (NULL == m_swapChain)	return NULL;
		ID3D11Texture2D* backBuffer;
		if (S_OK != m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)))
			return NULL;
		return backBuffer;*/
	}
}

bool zRender::DxRender_D3D11::isRenderTargetSetted()
{
	return m_bkbufRT != NULL || m_renderTargetTexture != NULL;
}

int zRender::DxRender_D3D11::resize( int new_width, int new_height )
{
	if(new_width<=0 || new_height<=0)
		return DXRENDER_RESULT_PARAM_INVALID;
	assert(m_context);
	assert(m_device);
	assert(m_bkbufRT || m_renderTargetTexture);
	if(m_context==NULL || NULL==m_device || !isRenderTargetSetted())
	{
		return DXRENDER_RESULT_NOT_INIT_YET;
	}
	// Release the old views, as they hold references to the buffers we
	// will be destroying.  Also release the old depth/stencil buffer.
	if (m_bkbufRT != nullptr)
	{
		m_bkbufRT->Resize(new_width, new_height, m_device);
		m_bkbufRT->SetRenderTarget(m_context);
	}
	// Set the viewport transform.

	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = static_cast<float>(new_width);
	m_viewport.Height = static_cast<float>(new_height);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_context->RSSetViewports(1, &m_viewport);
	return 0;

	/*

	ID3D11Texture2D* backBuffer = NULL;
	D3D11_TEXTURE2D_DESC textureDesc;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	if(NULL!=m_hWnd)
	{
		// Resize the swap chain and recreate the render target view.
		DXGI_SWAP_CHAIN_DESC sd_desc;
		m_swapChain->GetDesc(&sd_desc);
		HRESULT rslt = m_swapChain->ResizeBuffers(0, new_width, new_height, DXGI_FORMAT_UNKNOWN, sd_desc.Flags);
		if(FAILED(rslt))
		{
			return -3;
		}
		if(FAILED(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer))))
		{
			return -4;
		}
		if(NULL==backBuffer)
		{
			return -5;
		}
		backBuffer->GetDesc(&textureDesc);
	}
	else
	{
		HRESULT result;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

		// Initialize the render target texture description.
		ZeroMemory(&textureDesc, sizeof(textureDesc));

		// Setup the render target texture description.
		textureDesc.Width = new_width;
		textureDesc.Height = new_height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		// Create the render target texture.
// 		result = m_device->CreateTexture2D(&textureDesc, NULL, &m_renderTargetBuffer);
// 		if(FAILED(result))
// 		{
// 			TCHAR errmsg[512] = {0};
// 			swprintf_s(errmsg, 512, L"Error in DxRender_D3D11::resize : create Texture2D for render target failed.");
// 			return -7;
// 		}
	}
*/
}

int zRender::DxRender_D3D11::createTextureResource(TextureResource ** ppOutTexRes, int width, int height, DXGI_FORMAT dxgiFmt, TEXTURE_USAGE usage, bool bShared, const char * initData, int dataLen, int pitch)
{
	if (ppOutTexRes == NULL)	return -1;
	TextureResource* frameTexture = new TextureResource();
	if (0 != frameTexture->create(m_device, width, height, dxgiFmt, usage, bShared, initData, dataLen, pitch))
	{
		delete frameTexture;
		return -2;
	}
	*ppOutTexRes = frameTexture;
	return 0;
}

int zRender::DxRender_D3D11::openSharedTextureResource(TextureResource ** ppOutTexRes, HANDLE hSharedRes)
{
	if (ppOutTexRes == NULL)	return -1;
	TextureResource* frameTexture = new TextureResource();
	if (0 != frameTexture->open(m_device, hSharedRes))
	{
		delete frameTexture;
	}
	*ppOutTexRes = frameTexture;
	return 0;
}

void zRender::DxRender_D3D11::releaseTextureResource(TextureResource ** ppOutTexRes)
{
	if (ppOutTexRes)
	{
		TextureResource* pTexRes = *ppOutTexRes;
		if (pTexRes)
		{
			printf("do DxRender_D3D11::releaseTextureResource\n");
			pTexRes->release();
			delete pTexRes;
			*ppOutTexRes = NULL;
		}
	}
}
