#include "inc/TextureResource.h"

using namespace zRender;

TextureResource::TextureResource()
	: m_width(0), m_height(0)
	, m_dxgifmt(DXGI_FORMAT_UNKNOWN), m_usage(TEXTURE_USAGE_DEFAULT)
	, m_texture(NULL), m_rsv(NULL)
	, m_device(NULL), m_contex(NULL)
	, m_bShared(false), m_sharedHandle(INVALID_HANDLE_VALUE)
	, m_resMutex(NULL)
{

}

TextureResource::~TextureResource()
{
	releaseResourceView();
	release();
}

int TextureResource::create(ID3D11Device* device, int width, int height, DXGI_FORMAT dxgifmt, TEXTURE_USAGE usage, bool bShared, const char* initData, int dataLen, int pitch)
{
	if (device == NULL || width <= 0 || height <= 0 || dxgifmt == DXGI_FORMAT_UNKNOWN)
		return -1;
	//TEXTURE_USAGE_DEFAULT != usage时不可以shared
	if (bShared && TEXTURE_USAGE_DEFAULT != usage)
		return -1;
	if (m_texture != NULL)
		return -2;
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.ArraySize = texDesc.MipLevels = 1;
	texDesc.Height = height;
	texDesc.Width = width;
	texDesc.Format = dxgifmt;
	texDesc.SampleDesc.Quality = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.BindFlags = usage == TEXTURE_USAGE_STAGE ? 0 : D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = usage==TEXTURE_USAGE_STAGE ? D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ : 0;
	texDesc.MiscFlags = bShared ? D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX : 0;
	if (usage == TEXTURE_USAGE_STAGE)
	{
		texDesc.Usage = D3D11_USAGE_STAGING;/*D3D11_USAGE_DYNAMIC*/
	}
	else
	{
		texDesc.Usage = D3D11_USAGE_DEFAULT;
	}

	D3D11_SUBRESOURCE_DATA* pSubResData = NULL;
	D3D11_SUBRESOURCE_DATA initSubData;
	ZeroMemory(&initSubData, sizeof(D3D11_SUBRESOURCE_DATA));
	if (initData && dataLen >= /*frameSize*/0)
	{
		initSubData.pSysMem = initData;
		initSubData.SysMemPitch = pitch;
		pSubResData = &initSubData;
	}
	HRESULT rslt = S_OK;
	if (FAILED(rslt = device->CreateTexture2D(&texDesc, pSubResData, &m_texture)))
	{
		return -2;
	}
	if (bShared)
	{
		getSharedHandleFromTexture();
	}
	m_width = width;
	m_height = height;
	m_dxgifmt = dxgifmt;
	m_device = device;
	m_device->GetImmediateContext(&m_contex);
	m_bShared = bShared;
	m_usage = usage;
	return 0;
}

int TextureResource::open(ID3D11Device* device, HANDLE sharedHandle)
{
	if (NULL == device || INVALID_HANDLE_VALUE == sharedHandle)
		return -1;
	if (m_texture != NULL)
		return -2;
	// Obtain handle to Sync Shared Surface created by Direct3D10.1 Device 1.
	ID3D11Resource* tempResource11 = NULL;
	//HRESULT hr = pOtherDevice->OpenSharedResource( m_hRes, m_resIID, ppOpendSharedResource);
	HRESULT hr = device->OpenSharedResource(sharedHandle, __uuidof(ID3D11Resource), (void**)(&tempResource11));
	if (FAILED(hr))
		return -2;
	//pDevice11->OpenSharedResource(sharedHandle, __uuidof(ID3D11Resource), (void**)(&tempResource11)); 
	hr = tempResource11->QueryInterface(/*m_resIID*/__uuidof(ID3D11Texture2D), (void**)(&m_texture));
	tempResource11->Release();
	if (FAILED(hr) || NULL==m_texture)
		return -3;
	if (NULL == m_resMutex)
	{
		if (FAILED(m_texture->QueryInterface(__uuidof(IDXGIKeyedMutex), (LPVOID*)&m_resMutex)) || NULL == m_resMutex)
		{
			return -4;
		}
		// 		acqKey = 1;
		// 		relKey = 0;
	}
	m_device = device;
	m_device->GetImmediateContext(&m_contex);
	D3D11_TEXTURE2D_DESC texDesc;
	m_texture->GetDesc(&texDesc);
	m_width = texDesc.Width;
	m_height = texDesc.Height;
	m_dxgifmt = texDesc.Format;
	m_bShared = true;
	switch (texDesc.Usage)
	{
	case D3D11_USAGE_STAGING:
		m_usage = TEXTURE_USAGE_STAGE;
		break;
	case D3D11_USAGE_DEFAULT:
		m_usage = TEXTURE_USAGE_DEFAULT;
		break;
	default:
		break;
	}
	return 0;
}

void TextureResource::release()
{
	if (m_resMutex)
	{
		ReleaseCOM(m_resMutex);
	}
	ReleaseCOM(m_texture);
}

int TextureResource::createResourceView()
{
	if (NULL == m_device || NULL == m_texture)
		return -1;
	D3D11_SHADER_RESOURCE_VIEW_DESC	srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = m_dxgifmt;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	if (FAILED(m_device->CreateShaderResourceView(m_texture, &srvDesc, &m_rsv)))
		return -2;
	return 0;
}

void TextureResource::releaseResourceView()
{
	ReleaseCOM(m_rsv);
}

int TextureResource::copyResource(const TextureResource* res)
{
	if (res == NULL || !res->valid() || !valid())
		return -1;
	if (isShared())
	{
		acquireSync(0, INFINITE);
	}
	//copy full texture
	D3D11_BOX box;
	box.front = 0;
	box.back = 1;
	box.left = 0;
	box.right = m_width;
	box.top = 0;
	box.bottom = m_height;
	m_contex->CopySubresourceRegion(m_texture, 0, 0, 0, 0, res->m_texture, 0, &box);
	if (isShared())
	{
		releaseSync(0);
	}
	return 0;
}

int zRender::TextureResource::copyTexture(ID3D11Texture2D * d3dTex2D)
{
	if (NULL == d3dTex2D)	return -1;
	if (NULL == m_texture) return -2;
	D3D11_TEXTURE2D_DESC srcDesc; 
	D3D11_TEXTURE2D_DESC dstDesc;
	d3dTex2D->GetDesc(&srcDesc);
	m_texture->GetDesc(&dstDesc);
	if (srcDesc.Width != dstDesc.Width || srcDesc.Height != dstDesc.Height || srcDesc.Format != dstDesc.Format)
		return -3;
	if (isShared())
	{
		acquireSync(0, INFINITE);
	}
	//fixme 可能需要考虑如果源Texture是共享的时候是否需要先acquireSync
	//copy full texture
	D3D11_BOX box;
	box.front = 0;
	box.back = 1;
	box.left = 0;
	box.right = srcDesc.Width;
	box.top = 0;
	box.bottom = srcDesc.Height;
	m_contex->CopySubresourceRegion(m_texture, 0, 0, 0, 0, d3dTex2D, 0, &box);

	if (isShared())
	{
		releaseSync(0);
	}
	return 0;
}

int TextureResource::update(const unsigned char* pData, int dataLen, int dataPitch, int width, int height, const RECT& regionUpdated)
{
	//fixme 暂时忽略regionUpdated参数
	if(NULL==pData || 0>=dataLen || 0>=width || 0>=height)
		return -1;
	if (NULL == m_texture || m_device==NULL || m_contex==NULL)
		return -2;
	//如果是DEFAULT类型的显存Texture，则不支持update
	//同时如果是共享显存则必须为DEFAULT类型，因此共享显存调用update也必将导致失败
	if (TEXTURE_USAGE_DEFAULT == m_usage)
		return -3;
	D3D11_TEXTURE2D_DESC desc = { 0 };
	m_texture->GetDesc(&desc);
	if (width != desc.Width || height != desc.Height)
		return -4;
	if (m_resMutex)
	{
		DWORD syncResult = m_resMutex->AcquireSync(0, INFINITE);
		if (syncResult != WAIT_OBJECT_0)
		{
			// Handle unable to acquire shared surface error.
			return -6;
		}
	}
	HRESULT rslt = S_FALSE;
	D3D11_MAPPED_SUBRESOURCE mappedRes;
	ZeroMemory(&mappedRes, sizeof(mappedRes));
	if (S_OK != (rslt = m_contex->Map(m_texture, 0, D3D11_MAP_WRITE, /*D3D11_MAP_FLAG_DO_NOT_WAIT*/0, &mappedRes)))
	{
		// 		TCHAR errmsg[1024] = {0};
		// 		swprintf_s(errmsg, 1024, L"YUVTexture_Planar::update : ID3D11DeviceContext obj is Not match to this Y-Texture.this=[%d] result=[%d] errmsg=[%s]\n",
		// 			(unsigned int)this, rslt, L"");
		// 		OutputDebugString(errmsg);
		// 		log_e(LOG_TAG, errmsg, 20);
		//ReleaseSync();
		return -5;
	}
	unsigned char* dst = (unsigned char*)mappedRes.pData;
	unsigned char* src = (unsigned char*)pData;
	int pitch = dataPitch < mappedRes.RowPitch ? dataPitch : mappedRes.RowPitch;
	for (UINT i = 0; i<desc.Height; i++)
	{
		memcpy(dst, src, pitch);
		dst += mappedRes.RowPitch;
		src += dataPitch;
	}
	m_contex->Unmap(m_texture, 0);
	if (m_resMutex)
	{
		m_resMutex->ReleaseSync(0);
	}
	return 0;
}

int zRender::TextureResource::acquireSync(int key, unsigned int timeout)
{
	if (m_resMutex)
	{
		DWORD syncResult = m_resMutex->AcquireSync(key, timeout);
		if (syncResult != WAIT_OBJECT_0)
		{
			// Handle unable to acquire shared surface error.
			return -6;
		}
	}
	return 0;
}

int zRender::TextureResource::releaseSync(int key)
{
	if (m_resMutex)
	{
		m_resMutex->ReleaseSync(0);
	}
	return 0;
}

#include "D3DX11tex.h"
#include <fstream>
bool zRender::TextureResource::dumpToFile(const TCHAR * filePathName)
{
	if (isShared())
	{
		TextureResource* stagingTex = new TextureResource();
		if (0 != stagingTex->create(m_device, m_width, m_height, m_dxgifmt, TEXTURE_USAGE_STAGE, false, NULL, 0, 0))
		{
			delete stagingTex;
			return false;
		}
		acquireSync(0, INFINITE);
		if (0 != stagingTex->copyTexture(m_texture))
		{
			releaseSync(0);
			delete stagingTex;
			return false;
		}
		bool ret = stagingTex->dumpToFile(filePathName);
		releaseSync(0);
		delete stagingTex;
		return ret;
	}
	else
	{
		if (m_usage == TEXTURE_USAGE_STAGE)
		{
			std::ofstream outFile(filePathName, std::ios::binary | std::ios::out | std::ios::ate | std::ios::app);
			if (!outFile)
			{
				return false;
			}
			HRESULT rslt = S_FALSE;
			D3D11_MAPPED_SUBRESOURCE mappedRes;
			ZeroMemory(&mappedRes, sizeof(mappedRes));
			if (S_OK != (rslt = m_contex->Map(m_texture, 0, D3D11_MAP_READ, /*D3D11_MAP_FLAG_DO_NOT_WAIT*/0, &mappedRes)))
			{
				return false;
			}
			unsigned char* dst = (unsigned char*)mappedRes.pData;
			int pitch = mappedRes.RowPitch;
			for (UINT i = 0; i<m_height; i++)
			{
				outFile.write((char*)dst, pitch);
				dst += mappedRes.RowPitch;
			}
			outFile.flush();
			outFile.close();
			m_contex->Unmap(m_texture, 0);
			return true;
		}
		//D3DX11SaveTextureToFile(m_contex, m_texture, D3DX11_IFF_BMP, filePathName);
	}
	return false;
}

bool zRender::TextureResource::dumpToBuffer(unsigned char * outBuffer, int * in_outBufferLen, int * outPitch)
{
	if (outBuffer == NULL || in_outBufferLen == NULL || outPitch == NULL)
		return false;
	if (isShared())
	{
		TextureResource* stagingTex = new TextureResource();
		if (0 != stagingTex->create(m_device, m_width, m_height, m_dxgifmt, TEXTURE_USAGE_STAGE, false, NULL, 0, 0))
		{
			delete stagingTex;
			return false;
		}
		acquireSync(0, INFINITE);
		if (0 != stagingTex->copyTexture(m_texture))
		{
			releaseSync(0);
			delete stagingTex;
			return false;
		}
		bool ret = stagingTex->dumpToBuffer(outBuffer, in_outBufferLen, outPitch);
		releaseSync(0);
		delete stagingTex;
		return ret;
	}
	else
	{
		if (m_usage == TEXTURE_USAGE_STAGE)
		{
			HRESULT rslt = S_FALSE;
			D3D11_MAPPED_SUBRESOURCE mappedRes;
			ZeroMemory(&mappedRes, sizeof(mappedRes));
			if (S_OK != (rslt = m_contex->Map(m_texture, 0, D3D11_MAP_READ, /*D3D11_MAP_FLAG_DO_NOT_WAIT*/0, &mappedRes)))
			{
				return false;
			}
			if (*in_outBufferLen < mappedRes.RowPitch*m_height)
			{
				m_contex->Unmap(m_texture, 0);
				return false;
			}
			unsigned char* src = (unsigned char*)mappedRes.pData;
			int pitch = mappedRes.RowPitch;
			for (UINT i = 0; i<m_height; i++)
			{
				memcpy(outBuffer, src, pitch);
				src += pitch;
				outBuffer += pitch;
			}
			m_contex->Unmap(m_texture, 0);
			*in_outBufferLen = mappedRes.RowPitch*m_height;
			*outPitch = pitch;
			return true;
		}
		//D3DX11SaveTextureToFile(m_contex, m_texture, D3DX11_IFF_BMP, filePathName);
	}
	return false;
}

void zRender::TextureResource::getSharedHandleFromTexture()
{
	// QI IDXGIResource interface to synchronized shared surface.
	IDXGIResource* pDXGIResource = NULL;
	if (FAILED(m_texture->QueryInterface(__uuidof(IDXGIResource), (LPVOID*)&pDXGIResource)))
	{
		return;
	}

	// obtain handle to IDXGIResource object.
	pDXGIResource->GetSharedHandle(&m_sharedHandle);
	pDXGIResource->Release();
	if (INVALID_HANDLE_VALUE==m_sharedHandle)
		return;
	//getAdapterName(pSrcDevice);

	if (!m_resMutex)
	{
		// 	// QI IDXGIKeyedMutex interface of synchronized shared surface's resource handle.
		if (FAILED(m_texture->QueryInterface(__uuidof(IDXGIKeyedMutex), (LPVOID*)&m_resMutex)) || NULL == m_resMutex)
		{
			return;
		}
	}
}
