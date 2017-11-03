#include "YUVTexture_NV12.h"
#include "DxRenderCommon.h"
#include "DXLogger.h"

using namespace std;
using namespace zRender;

const float factorToFrameBufLen_nv12 = 1.5;

#define FrameSize_NV12(x, y) ((x)*(y)*(factorToFrameBufLen_nv12))
#define LOG_TAG L"D3D11_YUVTexture_NV12"

YUVTexture_NV12::YUVTexture_NV12(PIXFormat pixfmt)
	: RawFrameTextureBase(pixfmt)
	, m_yTex(NULL), m_uvTex(NULL)
	, m_ySRV(NULL), m_uvSRV(NULL)
{
	//YUVTexture_NV12类型的Texture只支持NV12这样内存布局的图片显示
	if(pixfmt!=PIXFMT_NV12)
		m_pixfmt = PIXFMT_UNKNOW;
}

YUVTexture_NV12::~YUVTexture_NV12()
{
	destroy();
}

int YUVTexture_NV12::create(ID3D11Device* device, int width, int height, const char* initData, int dataLen)
{
	if(PIXFMT_UNKNOW==m_pixfmt)
	{
#ifdef _DEBUG
		printf("Error in YUVTexture_NV12::create : pixel format is Not support.(PIXFMT=%d)\n", m_pixfmt);
#endif
		return -1;
	}
	if(device==NULL || width<=0 || height<=0)
		return -1;
	int frameSize = (int)FRAMESIZE(width, height, m_pixfmt);
	if(frameSize<=0)
		return -2;
	
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.ArraySize = texDesc.MipLevels = 1;
	texDesc.Height = height;
	texDesc.Width = width;
	texDesc.Format = DXGI_FORMAT_R8_UNORM;
	texDesc.SampleDesc.Quality = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	texDesc.MiscFlags = 0;
	texDesc.Usage = D3D11_USAGE_DYNAMIC;

	D3D11_SUBRESOURCE_DATA* pSubResData = NULL;
	D3D11_SUBRESOURCE_DATA initSubData;
	ZeroMemory(&initSubData, sizeof(D3D11_SUBRESOURCE_DATA));
	if(initData && dataLen>=frameSize)
	{
		initSubData.pSysMem = initData;
		initSubData.SysMemPitch = width;
		pSubResData = &initSubData;
	}
	if(FAILED(device->CreateTexture2D(&texDesc, pSubResData, &m_yTex)))
		goto ErrorEnd;

	texDesc.Height = height / 2;
	texDesc.Width = width / 2;
	texDesc.Format = DXGI_FORMAT_R8G8_UNORM;
	ZeroMemory(&initSubData, sizeof(D3D11_SUBRESOURCE_DATA));
	if(initData && dataLen>=frameSize)
	{
		initSubData.pSysMem = initData + width * height;
		initSubData.SysMemPitch = width;
	}
	if(FAILED(device->CreateTexture2D(&texDesc, pSubResData, &m_uvTex)))
		goto ErrorEnd;

	D3D11_SHADER_RESOURCE_VIEW_DESC	srvDesc;
	ZeroMemory(&srvDesc,sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	if(FAILED(device->CreateShaderResourceView(m_yTex, &srvDesc, &m_ySRV)))
		goto ErrorEnd;

	srvDesc.Format = DXGI_FORMAT_R8G8_UNORM;
	if(FAILED(device->CreateShaderResourceView(m_uvTex, &srvDesc, &m_uvSRV)))
		goto ErrorEnd;

	m_device = device;
	m_width = width;
	m_height = height;
	return 0;
ErrorEnd:
	destroy();
	return -3;
}

int YUVTexture_NV12::create(ID3D11Device * device, int width, int height, TEXTURE_USAGE usage, bool bShared, const char * initData, int dataLen, int pitch)
{
	return -1;
}

int YUVTexture_NV12::destroy()
{
	ReleaseCOM(m_ySRV);
	ReleaseCOM(m_uvSRV);
	ReleaseCOM(m_yTex);
	ReleaseCOM(m_uvTex);
	m_device = NULL;
	m_width = 0;
	m_height = 0;
	return 0;
}

int YUVTexture_NV12::getTexture(ID3D11Texture2D** outYUVTexs, int& texsCount) const
{
	if(texsCount<2 || outYUVTexs==NULL)
		return -1;
	if(m_yTex==NULL || NULL==m_uvTex || m_ySRV==NULL || m_uvSRV==NULL)
		return -2;

	*(outYUVTexs+0) = m_yTex;
	*(outYUVTexs+1) = m_uvTex;
	texsCount = 2;

	return 0;
}

int YUVTexture_NV12::getShaderResourceView(ID3D11ShaderResourceView** outYUVSRVs, int& srvsCount) const
{
	if(srvsCount<2 || outYUVSRVs==NULL)
		return -1;
	if(m_yTex==NULL || NULL==m_uvTex || m_ySRV==NULL || m_uvSRV==NULL)
		return -2;

	*(outYUVSRVs+0) = m_ySRV;
	*(outYUVSRVs+1) = m_uvSRV;
	srvsCount = 2;

	return 0;
}

int YUVTexture_NV12::update(const unsigned char* pData, int dataLen, int yPitch, int uPitch, int vPitch, int width, int height,
							const RECT& regionUpdated, ID3D11DeviceContext* d3dDevContex)
{
	if(PIXFMT_UNKNOW==m_pixfmt || m_yTex==NULL || m_uvTex==NULL || NULL==m_ySRV || NULL==m_uvSRV)
	{
#ifdef _DEBUG
		printf("Error in YUVTexture_NV12::update : Texture Not init yet.(PIXFMT=%d yTex=%d uvTex=%d ySRV=%d uvSRV=%d)\n", 
			m_pixfmt, (int)m_yTex, (int)m_uvTex, (int)m_ySRV, (int)m_uvSRV);
#endif
		return -1;
	}
	if(width<=0 || height<=0 || pData==NULL /*|| dataPitch<FRAMEPITCH(width, m_pixfmt)*/
		/*|| dataLen < dataPitch*height*/ || d3dDevContex==NULL)
	{
#ifdef _DEBUG
		printf("Error in YUVTexture_NV12::update : param invalid.(Data=%d dataLen=%d Pitch=%d uvPitch=%d width=%d height=%d Ctx=%d)\n",
			pData, dataLen, yPitch, uPitch, width, height, (int)d3dDevContex);
#endif
		return -2;
	}
	int regWidth = regionUpdated.right - regionUpdated.left;
	int regHeight = regionUpdated.bottom - regionUpdated.top;
	if(regionUpdated.left>=width || regionUpdated.top>=height || regionUpdated.left<0 || regionUpdated.top<0
		|| regionUpdated.right>width || regionUpdated.bottom>height || regWidth<=0 || regHeight<=0)
	{
#ifdef _DEBUG
		printf("Error in YUVTexture_NV12::update : Update region is invalid.(L=%d R=%d T=%d B=%d)\n",
			regionUpdated.left, regionUpdated.right, regionUpdated.top, regionUpdated.bottom);
		return -3;
#endif
	}

	int updatedWidth = regWidth >= m_width ? m_width : regWidth;
	int updatedHeight = regHeight >= m_height ? m_height : regHeight;
	int startPosHrz = regionUpdated.left;
	int startPosVtc = regionUpdated.top;
	unsigned char* startDataPos = NULL;

	D3D11_MAPPED_SUBRESOURCE mappedRes;
	ZeroMemory(&mappedRes, sizeof(mappedRes));
	D3D11_TEXTURE2D_DESC texDesc;
	m_yTex->GetDesc(&texDesc);
	HRESULT rslt = S_OK;
	if(S_OK!=(rslt=d3dDevContex->Map(m_yTex, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes)))
	{
#ifdef _DEBUG
		printf("Error in Error in YUVTexture_NV12::update : ID3D11DeviceContext obj is Not match to this Y-Texture.\n");
#endif
		TCHAR errmsg[1024] = {0};
		swprintf_s(errmsg, 1024, L"YUVTexture_NV12::update : ID3D11DeviceContext obj is Not match to this Y-Texture.this=[%d] result=[%d] errmsg=[%s]\n",
			(unsigned int)this, rslt, L"");
		OutputDebugString(errmsg);
		log_e(LOG_TAG, errmsg);
		return -4;
	}
	startDataPos = (unsigned char*)pData + (startPosVtc * width + startPosHrz);
	unsigned char* pDes = (unsigned char*)mappedRes.pData;
	for(int iVtc=0; iVtc<updatedHeight; iVtc++)
	{				
		memcpy(pDes, startDataPos, updatedWidth);
		pDes += mappedRes.RowPitch;
		startDataPos += width;
	}
	d3dDevContex->Unmap(m_yTex, 0);

	if(S_OK!=(rslt=d3dDevContex->Map(m_uvTex, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes)))
	{
#ifdef _DEBUG
		printf("Error in Error in YUVTexture_NV12::update : ID3D11DeviceContext obj is Not match to this U-Texture.\n");
#endif
		TCHAR errmsg[1024] = {0};
		swprintf_s(errmsg, 1024, L"YUVTexture_Packed::update : ID3D11DeviceContext obj is Not match to this UV-Texture.this=[%d] result=[%d] errmsg=[%s]\n",
			(unsigned int)this, rslt, L"");
		OutputDebugString(errmsg);
		log_e(LOG_TAG, errmsg);
		return -5;
	}

	startDataPos = (unsigned char*)pData + ((width * height) + ((startPosVtc/2) * width + (startPosHrz / 2) * 2));

	pDes = (unsigned char*)mappedRes.pData;
	int byteCopyed = (updatedWidth + 1) / 2 * 2;
	for(int iVtc=0; iVtc<updatedHeight/2; iVtc++)
	{				
		memcpy(pDes, startDataPos, byteCopyed );//updatedWidth & 0xFFFFFFFE
		pDes += mappedRes.RowPitch;
		startDataPos += width;
	}
	d3dDevContex->Unmap(m_uvTex, 0);
	return 0;
}

int zRender::YUVTexture_NV12::update(SharedTexture* pSharedTexture, const RECT& regionUpdated, ID3D11DeviceContext* d3dDevContex)
{
	return -1;
}
