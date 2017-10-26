#include "SharedFrameTexture.h"

using namespace zRender;

zRender::SharedResource::SharedResource(HANDLE hRes, REFIID resIID)
: m_resIID(resIID)
, m_hRes(hRes)
{

}

SharedResource::SharedResource()
: m_hRes(INVALID_HANDLE_VALUE)
, m_fmt(DXGI_FORMAT_UNKNOWN)
, m_textureRes(NULL)
, m_resIID(__uuidof(ID3D11Texture2D))
, m_resMutex(NULL)
{

}

SharedResource::~SharedResource()
{
	if(m_textureRes)
	{
		m_textureRes->Release();
		m_textureRes = NULL;
	}
	ReleaseCOM(m_resMutex);
	m_fmt = DXGI_FORMAT_UNKNOWN;
}

int SharedResource::create(ID3D11Device* pSrcDevice, ID3D11DeviceContext* devContex, DXGI_FORMAT fmt, int width, int height, int usage, const unsigned char* initData, int dataLen)
{
	D3D11_TEXTURE2D_DESC texDescStage;
	texDescStage.ArraySize = texDescStage.MipLevels = 1;
	texDescStage.Height = height;
	texDescStage.Width = width;
	texDescStage.Format = fmt;
	texDescStage.SampleDesc.Quality = 0;
	texDescStage.SampleDesc.Count = 1;
	texDescStage.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDescStage.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	texDescStage.MiscFlags = /*D3D11_RESOURCE_MISC_SHARED*/D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
	texDescStage.Usage = (D3D11_USAGE)usage/*D3D11_USAGE_STAGING*/;

	D3D11_SUBRESOURCE_DATA* pSubResData = NULL;
	D3D11_SUBRESOURCE_DATA initSubData;
	ZeroMemory(&initSubData, sizeof(D3D11_SUBRESOURCE_DATA));
	if(initData && dataLen>=0)
	{
		initSubData.pSysMem = initData;
		initSubData.SysMemPitch = dataLen / height;//fix me pitch maybe incorrect
		pSubResData = &initSubData;
	}
	HRESULT rslt = S_OK;
	ID3D11Texture2D* texStage = NULL;
	if(FAILED(rslt = pSrcDevice->CreateTexture2D(&texDescStage, pSubResData, &texStage)))
		return -2;
	m_textureRes = texStage;
// 	D3D11_MAPPED_SUBRESOURCE mappedRes;
// 	ZeroMemory(&mappedRes, sizeof(mappedRes));
// 	if(S_OK!=(rslt=devContex->Map(texStage, 0, D3D11_MAP_WRITE, /*D3D11_MAP_FLAG_DO_NOT_WAIT*/0, &mappedRes)))
// 	{
// // 		TCHAR errmsg[1024] = {0};
// // 		swprintf_s(errmsg, 1024, L"YUVTexture_Planar::update : ID3D11DeviceContext obj is Not match to this Y-Texture.this=[%d] result=[%d] errmsg=[%s]\n",
// // 			(unsigned int)this, rslt, L"");
// // 		OutputDebugString(errmsg);
// // 		log_e(LOG_TAG, errmsg, 20);
// 		return -3;
// 	}
// 	int src_pitch = dataLen / height;
// 	unsigned char* dst = (unsigned char*)mappedRes.pData;
// 	unsigned char* src = (unsigned char*)initData;
// 	for (UINT i=0; i<texDescStage.Height; i++)
// 	{
// 		memcpy(dst, src, src_pitch);
// 		dst += mappedRes.RowPitch;
// 		src += src_pitch;
// 	}
// 	devContex->Unmap(texStage, 0);

	// QI IDXGIResource interface to synchronized shared surface.
	IDXGIResource* pDXGIResource = NULL;
	if(FAILED(texStage->QueryInterface(__uuidof(IDXGIResource), (LPVOID*)&pDXGIResource)))
	{
		return -3;
	}

	// obtain handle to IDXGIResource object.
	pDXGIResource->GetSharedHandle(&m_hRes);
	pDXGIResource->Release();
	if ( !m_hRes )
		return -4;
	m_fmt = fmt;
	getAdapterName(pSrcDevice);

 	if(!m_resMutex)
 	{
 	// 	// QI IDXGIKeyedMutex interface of synchronized shared surface's resource handle.
 		if(FAILED(m_textureRes->QueryInterface( __uuidof(IDXGIKeyedMutex), (LPVOID*)&m_resMutex )) || NULL==m_resMutex)
 		{
 			return -5;
 		}
 		acqKey = 0;
 		relKey = 0;
 	}
// 	If ( FAILED( hr ) || ( g_pDXGIKeyedMutex_dev1 == NULL ) )
// 		return E_FAIL;
	return 0;
}

int SharedResource::open(ID3D11Device* pOtherDevice, void** ppOpendSharedResource, IDXGIKeyedMutex** keyMutex)
{
	if(NULL==pOtherDevice)
		return -1;
	if(INVALID_HANDLE_VALUE==m_hRes/* || DXGI_FORMAT_UNKNOWN==m_fmt*/)
		return -2;
	// Obtain handle to Sync Shared Surface created by Direct3D10.1 Device 1.
	ID3D11Resource* tempResource11 = NULL;
	//HRESULT hr = pOtherDevice->OpenSharedResource( m_hRes, m_resIID, ppOpendSharedResource);
	HRESULT hr = pOtherDevice->OpenSharedResource( m_hRes, __uuidof(ID3D11Resource), (void**)(&tempResource11)); 
	if (FAILED (hr))
		return -3;
	//pDevice11->OpenSharedResource(sharedHandle, __uuidof(ID3D11Resource), (void**)(&tempResource11)); 
	hr = tempResource11->QueryInterface(/*m_resIID*/__uuidof(ID3D11Texture2D), (void**)(ppOpendSharedResource)); 
	tempResource11->Release();
	if (FAILED (hr))
		return -4;
	if(NULL!=keyMutex && NULL==*keyMutex)
	{
		if(FAILED(tempResource11->QueryInterface( __uuidof(IDXGIKeyedMutex), (LPVOID*)keyMutex )) || NULL==keyMutex)
		{
			return -5;
		}
// 		acqKey = 1;
// 		relKey = 0;
	}
	/*hr = g_pdev2Shared->QueryInterface( __uuidof(IDXGIKeyedMutex),
		(LPVOID*) &g_pDXGIKeyedMutex_dev2);
	if( FAILED( hr ) || ( g_pDXGIKeyedMutex_dev2 == NULL ) )
		return E_FAIL;

	// Rendering onto Sync Shared Surface from D3D10.1 Device 1 using D3D10.1 Device 2.
	UINT acqKey = 1;
	UINT relKey = 0;
	DWORD timeOut = 5;
	DWORD result = g_pDXGIKeyedMutex_dev2->AcquireSync(acqKey, timeOut);
	if ( result == WAIT_OBJECT_0 )
		// Rendering calls using Device 2.
	else
	// Handle unable to acquire shared surface error.
	result = g_pDXGIKeyedMutex_dev2->ReleaseSync(relKey));
	if (result == WAIT_OBJECT_0)
		return S_OK;*/
	return 0;
}


//
// D3D9Ex does not have an API to get shared handles.  We replicate that functionality
// using setprivatedata.
//
static GUID SharedHandleGuid = {0x91facf2d, 0xe464, 0x4495, 0x84, 0xa6, 0x37, 0xbe, 0xd3, 0x56, 0x8d, 0xa3};
//
// This function will convert from DXGI formats (d3d10/d3d11) to D3D9 formats.
// Most formtas are not cross api shareable and for those the function will
// return D3DFMT_UNKNOWN.
//
D3DFORMAT DXGIToCrossAPID3D9Format(DXGI_FORMAT Format)
{
	switch (Format)
	{
	case DXGI_FORMAT_B8G8R8A8_UNORM:
		return D3DFMT_A8R8G8B8;
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: 
		return D3DFMT_A8R8G8B8;
	case DXGI_FORMAT_B8G8R8X8_UNORM:
		return D3DFMT_X8R8G8B8;
	case DXGI_FORMAT_R8G8B8A8_UNORM: 
		return D3DFMT_A8B8G8R8;
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: 
		return D3DFMT_A8B8G8R8;
	case DXGI_FORMAT_R10G10B10A2_UNORM:
		return D3DFMT_A2B10G10R10;
	case DXGI_FORMAT_R16G16B16A16_FLOAT: 
		return D3DFMT_A16B16G16R16F;
	default:
		return D3DFMT_UNKNOWN;
	};
}

#include "D3dx9tex.h"
#include <fstream>
int zRender::SharedResource::open(IDirect3DDevice9Ex* pOtherDevice, void** ppOpendSharedResource)
{
	if(pOtherDevice==NULL || ppOpendSharedResource==NULL)
		return -1;
	D3DFORMAT D3D9Format;

	// If the format is not cross api shareable the utility function will return
	// D3DFMT_UNKNOWN
	if ((D3D9Format = DXGIToCrossAPID3D9Format(m_fmt)) == D3DFMT_UNKNOWN)
	{
		return -2;
	}

	HRESULT hr = S_OK;
	IDirect3DTexture9** ppTexture = (IDirect3DTexture9**)ppOpendSharedResource;

	D3D11_TEXTURE2D_DESC texDesc;
	m_textureRes->GetDesc(&texDesc);
	hr = pOtherDevice->CreateTexture(texDesc.Width, texDesc.Height, 1,
		D3DUSAGE_RENDERTARGET,
		D3D9Format,
		D3DPOOL_DEFAULT,
		ppTexture,
		&m_hRes);
	if (SUCCEEDED(hr))
	{
		// Store the shared handle
		hr = (*ppTexture)->SetPrivateData(SharedHandleGuid, &m_hRes, sizeof(HANDLE), 0);

		if (FAILED(hr))
		{
			(*ppTexture)->Release();
			*ppTexture = NULL;
		}
	}
	else
	{
		return -4;
	}

//	hr = D3DXSaveTextureToFile(L"dump_snapshot_tex.bmp", D3DXIFF_BMP, *ppTexture, NULL);
// 	char tmpMem[2048] = {0};
 	D3DLOCKED_RECT lockedRect;
 	if(FAILED(hr=(*ppTexture)->LockRect(0, &lockedRect, NULL, D3DLOCK_DISCARD)))
 	{
 		return -3;
 	}
	const unsigned char* source = static_cast< const unsigned char* >(lockedRect.pBits);
	static std::ofstream* imgfile = NULL;
	static unsigned int pThis = 0;
	if(imgfile==NULL)
	{
		pThis = (unsigned int)pOtherDevice;
		char strFileName[128] = { 0 };
		sprintf(strFileName, "outfile_%d_%d_%u.rgb", texDesc.Width, texDesc.Height, pThis);
		imgfile = new std::ofstream(strFileName, std::ios::out | std::ios::binary);
	}
	else if((unsigned int)pOtherDevice==pThis)
	{
		for (size_t i = 0; i < texDesc.Height; ++i)
		{
			const unsigned char* psrc = source + i*lockedRect.Pitch;
			imgfile->write((const char*)psrc, lockedRect.Pitch);
		}
	}
 
 	(*ppTexture)->UnlockRect(0);

	return 0;
}

void zRender::SharedResource::getAdapterName(ID3D11Device* pSrcDevice)
{
	IDXGIDevice* dxgiDev = NULL;
	if(FAILED(pSrcDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDev)) || dxgiDev==NULL)
	{
		return;
	}
	IDXGIAdapter *pAdapter = NULL;
	if(FAILED(dxgiDev->GetAdapter(&pAdapter)) || NULL==pAdapter)
	{
		dxgiDev->Release();
		return;
	}
	DXGI_ADAPTER_DESC pDesc;
	if(FAILED(pAdapter->GetDesc(&pDesc)))
	{
		return;
	}
	m_strAdapter = pDesc.Description;
}

REFIID zRender::SharedResource::getTextureTypeID() const
{ 
	return m_resIID;
}

int zRender::SharedResource::update( ID3D11Device* pSrcDevice, ID3D11DeviceContext* devContex, int width, int height, const unsigned char* initData, int dataLen, int pitch )
{
	if(NULL==m_textureRes)
		return -1;
	D3D11_TEXTURE2D_DESC desc = {0};
	m_textureRes->GetDesc(&desc);
	if(width!=desc.Width || height!=desc.Height)
		return -2;
	DWORD syncResult = AcquireSync(5);
	if ( syncResult != WAIT_OBJECT_0 )
	{
		// Handle unable to acquire shared surface error.
		return -4;
	}
	HRESULT rslt = S_FALSE;
 	D3D11_MAPPED_SUBRESOURCE mappedRes;
 	ZeroMemory(&mappedRes, sizeof(mappedRes));
 	if(S_OK!=(rslt=devContex->Map(m_textureRes, 0, D3D11_MAP_WRITE, /*D3D11_MAP_FLAG_DO_NOT_WAIT*/0, &mappedRes)))
 	{
 // 		TCHAR errmsg[1024] = {0};
 // 		swprintf_s(errmsg, 1024, L"YUVTexture_Planar::update : ID3D11DeviceContext obj is Not match to this Y-Texture.this=[%d] result=[%d] errmsg=[%s]\n",
 // 			(unsigned int)this, rslt, L"");
 // 		OutputDebugString(errmsg);
 // 		log_e(LOG_TAG, errmsg, 20);
		ReleaseSync();
 		return -3;
 	}
 	unsigned char* dst = (unsigned char*)mappedRes.pData;
 	unsigned char* src = (unsigned char*)initData;
 	for (UINT i=0; i<desc.Height; i++)
 	{
 		memcpy(dst, src, pitch);
 		dst += mappedRes.RowPitch;
 		src += pitch;
 	}
 	devContex->Unmap(m_textureRes, 0);
	ReleaseSync();
	return 0;
}

DWORD zRender::SharedResource::AcquireSync(DWORD timeout)
{
	if(m_resMutex)	return m_resMutex->AcquireSync(acqKey, timeout);
	return WAIT_OBJECT_0;
}

DWORD zRender::SharedResource::ReleaseSync()
{
	if(m_resMutex)	return m_resMutex->ReleaseSync(relKey);
	return WAIT_OBJECT_0;
}

SharedTexture_I420P::SharedTexture_I420P(ID3D11Device* pSrcDevice, ID3D11DeviceContext* devContex)
: m_resCount(0)
, m_pSrcDevice(pSrcDevice)
, m_devContext(devContex)
, m_pixfmt(PIXFMT_UNKNOW), m_width(0), m_height(0)
{
	m_resList[0] = m_resList[1] = m_resList[2] = NULL;
}

SharedTexture_I420P::~SharedTexture_I420P()
{
	release();
}

int SharedTexture_I420P::create(PIXFormat fmt, int width, int height, const unsigned char* initData, int dataLen, int yPitch, int uvPitch)
{
	if(m_pSrcDevice==NULL)
		return -1;
	if(0>width || 0>height || fmt==PIXFMT_UNKNOW)
		return -2;
	SharedResource* yRes = new SharedResource();
	SharedResource* uRes = new SharedResource();
	SharedResource* vRes = new SharedResource();
	const unsigned char	*initData_y = NULL, *initData_u = NULL, *initData_v = NULL;
	int yDataLen = 0, uDataLen = 0, vDataLen = 0;
	if(initData && yPitch>=width && uvPitch>=((width+1)>>1) && dataLen>=(yPitch*height + uvPitch*((height+1)>>1)))
	{
		//计算Y、U、V数据的起点位置以及长度
		initData_y = initData;
		yDataLen = yPitch * height;
		initData_u = initData + yDataLen;
		uDataLen = uvPitch * ((height+1)>>1);
		initData_v = initData + yDataLen + uDataLen;
		vDataLen = uDataLen;
	}
	if(0!=yRes->create(m_pSrcDevice, m_devContext, DXGI_FORMAT_R8_UNORM, width, height, (int)D3D11_USAGE_STAGING, initData_y, yDataLen))
	{
		goto failed_end;
	}
	if(0!=uRes->create(m_pSrcDevice, m_devContext, DXGI_FORMAT_R8_UNORM, (width+1)>>1, (height+1)>>1, (int)D3D11_USAGE_STAGING, initData_u, uDataLen))
	{
		goto failed_end;
	}
	if(0!=vRes->create(m_pSrcDevice, m_devContext, DXGI_FORMAT_R8_UNORM, (width+1)>>1, (height+1)>>1, (int)D3D11_USAGE_STAGING, initData_v, vDataLen))
	{
		goto failed_end;
	}
	m_resList[0] = yRes;
	m_resList[1] = uRes;
	m_resList[2] = vRes;
	m_resCount = 3;
	m_pixfmt = fmt;
	m_width = width;
	m_height = height;
	return 0;
failed_end:
	delete yRes;
	delete uRes;
	delete vRes;
	return -3;
}

int SharedTexture_I420P::release()
{
	if(m_resCount>0)
	{
		delete m_resList[0];
		m_resList[0] = NULL;
		delete m_resList[1];
		m_resList[1] = NULL;
		delete m_resList[2];
		m_resList[2] = NULL;
		m_resCount = 0;
		m_pixfmt = PIXFMT_UNKNOW;
		m_width = 0;
		m_height = 0;
	}
	else
	{
		return -1;
	}
	return 0;
}

int zRender::SharedTexture_I420P::update( PIXFormat fmt, int width, int height, const unsigned char* initData, int dataLen, int yPitch, int uvPitch )
{
	if(m_pSrcDevice==NULL || m_resCount<=0 || m_resList[2]==NULL)
		return -1;
	if(0>width || 0>height || fmt==PIXFMT_UNKNOW || 0>=dataLen || NULL==initData || width>yPitch)
		return -2;
	const unsigned char	*initData_y = NULL, *initData_u = NULL, *initData_v = NULL;
	int yDataLen = 0, uDataLen = 0, vDataLen = 0;
// 	if(initData && yPitch>=width && uvPitch>=((width+1)>>1) && dataLen>=(yPitch*height + uvPitch*((height+1)>>1)))
// 	{
		//计算Y、U、V数据的起点位置以及长度
		initData_y = initData;
		yDataLen = yPitch * height;
		initData_u = initData + yDataLen;
		uDataLen = uvPitch * ((height+1)>>1);
		initData_v = initData + yDataLen + uDataLen;
		vDataLen = uDataLen;
//	}
		SharedResource* yRes = m_resList[0];
		SharedResource* uRes = m_resList[1];
		SharedResource* vRes = m_resList[2];
		yRes->update(m_pSrcDevice, m_devContext, width, height, initData_y, yDataLen, yPitch);
		uRes->update(m_pSrcDevice, m_devContext, (width+1)>>1, (height+1)>>1, initData_u, uDataLen, uvPitch);
		vRes->update(m_pSrcDevice, m_devContext, (width+1)>>1, (height+1)>>1, initData_v, vDataLen, uvPitch);
		return 0;
}

bool zRender::SharedTexture_I420P::getDescription( PIXFormat& fmt, int& width, int& height )
{
	fmt = m_pixfmt;
	width = m_width;
	height = m_height;
	return true;
}

int SharedTexture_I420P::getSharedResource(SharedResource** ppSharedRes, int& count)
{
	if(ppSharedRes==NULL)	return -1;
	if(m_resCount<=0)
	{
		count = 0;
		*ppSharedRes = NULL;
		return 0;
	}
	count = m_resCount;
	*ppSharedRes = m_resList[0];
	ppSharedRes[1] = m_resList[1];
	ppSharedRes[2] = m_resList[2];
	return m_resCount;
}

zRender::SharedTexture_ARGB8::SharedTexture_ARGB8(ID3D11Device* pSrcDevice, ID3D11DeviceContext* devContex)
{

}

zRender::SharedTexture_ARGB8::~SharedTexture_ARGB8()
{

}

int zRender::SharedTexture_ARGB8::create(PIXFormat fmt, int width, int height, const unsigned char* initData, int dataLen, int yPitch, int uvPitch)
{
	return -1;
}

int zRender::SharedTexture_ARGB8::release()
{
	return -1;
}

int zRender::SharedTexture_ARGB8::getSharedResource(SharedResource** ppSharedRes, int& count)
{
	return -1;
}

int zRender::SharedTexture_ARGB8::update( PIXFormat fmt, int width, int height, const unsigned char* initData, int dataLen, int yPitch, int uvPitch )
{
	return -1;
}

bool zRender::SharedTexture_ARGB8::getDescription( PIXFormat& fmt, int& width, int& height )
{
	return false;
}
