#include "DxRender_D3D9.h"

using namespace zRender;

zRender::DxRender_D3D9::DxRender_D3D9()
: m_d3d9Obj(NULL)
, m_device(NULL)
{

}

int zRender::DxRender_D3D9::setVisibleRegion(const RECT_f& visibleReg)
{
	return 0;
}

RECT_f zRender::DxRender_D3D9::getVisibleREgion() const
{
	return m_visibleReg;
}

int zRender::DxRender_D3D9::init(HWND hWnd, const wchar_t* effectFileName, bool isEnable4XMSAA /*= false*/, bool isSDICompatible /*= false*/)
{
	LPDIRECT3D9EX pD3D = NULL;
	if( FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &pD3D)) || NULL==pD3D)
		return -1;
	LPDIRECT3DDEVICE9EX pDevice = NULL;
	D3DPRESENT_PARAMETERS d3dpp; 
	D3DDISPLAYMODEEX d3dDsplMod;

	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.Windowed   = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferCount = 1;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.hDeviceWindow = hWnd;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	if( FAILED( pD3D->CreateDeviceEx( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, /*&d3dDsplMod*/NULL, &pDevice ) ) )
	{
		return -2;
	}
	m_d3d9Obj = pD3D;
	m_device = pDevice;
	return 0;
}

int zRender::DxRender_D3D9::init(HMONITOR hmonitor)
{
	return -1;
}

void zRender::DxRender_D3D9::deinit()
{

}

DisplayElement* zRender::DxRender_D3D9::createDisplayElement(const RECT_f& displayReg, int zIndex)
{
	return NULL;
}

int zRender::DxRender_D3D9::releaseDisplayElement(DisplayElement** displayElement)
{
	return -1;
}

int zRender::DxRender_D3D9::createSharedTexture(SharedTexture** ppSharedTex, PIXFormat pixfmt)
{
	return -1;
}

int zRender::DxRender_D3D9::releaseSharedTexture(SharedTexture** ppSharedTex)
{
	return -1;
}

IRawFrameTexture* zRender::DxRender_D3D9::createTexture(PIXFormat pixFmt, int width, int height, unsigned char* initData /*= NULL*/, int initDataLen /*= 0*/, bool isShared /*= false*/)
{
	return NULL;
}

int zRender::DxRender_D3D9::releaseTexture(IRawFrameTexture** texture)
{
	return -1;
}

int zRender::DxRender_D3D9::resize(int new_width, int new_height)
{
	return 0;
}
