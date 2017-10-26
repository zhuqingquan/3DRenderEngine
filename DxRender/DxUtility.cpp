#include "DxUtility.h"
#include <Windows.h>
#include <d3d11.h>
#include <xnamath.h>
#include "DXLogger.h"
#include <tchar.h>

using namespace zRender;

#define LOG_TAG _T("DXUtility")

#ifdef _WINDOWS

zRender::RenderFeatureLevel getSupportFeatureLevel_Windows()
{
	UINT createDeviceFlags = 0;
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

	ID3D11Device* device = NULL;
	ID3D11DeviceContext* context = NULL;
	HRESULT hr = D3D11CreateDevice(
		NULL,                 // use selected adapter
		D3D_DRIVER_TYPE_HARDWARE,	//adapter is No NULL
		0,							// no software device
		createDeviceFlags, 
		featureLvs, ARRAYSIZE(featureLvs),// default feature level array
		D3D11_SDK_VERSION,
		&device,
		&curFeatureLv,
		&context);
	if(S_OK!=hr)
	{
		TCHAR errmsg[512] = {0};
		swprintf_s(errmsg, 512, L"Error in getSupportFeatureLevel_Windows : faile to Create device with param  Adapter(%d) CreateFlag(%d) ErrorCode=%d",
			 (int)NULL, createDeviceFlags, (int)hr);
		zRender::log_e(LOG_TAG, errmsg);
		return RENDER_FEATURE_LEVEL_UNKNOW;
	}
	switch(curFeatureLv)
	{
	case D3D_FEATURE_LEVEL_9_1:
		return RENDER_FEATURE_LEVEL_DX9_1;
	case D3D_FEATURE_LEVEL_9_2:
		return RENDER_FEATURE_LEVEL_DX9_2;
	case D3D_FEATURE_LEVEL_9_3:
		return RENDER_FEATURE_LEVEL_DX9_3;
	case D3D_FEATURE_LEVEL_10_0:
		return RENDER_FEATURE_LEVEL_DX10_0;
	case D3D_FEATURE_LEVEL_10_1:
		return RENDER_FEATURE_LEVEL_DX10_1;
	case D3D_FEATURE_LEVEL_11_0:
		return RENDER_FEATURE_LEVEL_DX11_0;
	default:
		return RENDER_FEATURE_LEVEL_UNKNOW;
	}
}

#endif

zRender::RenderFeatureLevel zRender::getSupportFeatureLevel()
{
#ifdef _WINDOWS
	return getSupportFeatureLevel_Windows();
#endif
}
