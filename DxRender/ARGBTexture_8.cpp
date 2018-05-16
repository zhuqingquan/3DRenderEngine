#include "ARGBTexture_8.h"
#include "DxRenderCommon.h"
#include <Windows.h>
#include <stdio.h>
#include "DXLogger.h"
#include "inc/TextureResource.h"

using namespace zRender;
#define LOG_TAG L"D3D11_ARGBTexture_8"

ARGBTexture_8::ARGBTexture_8(PIXFormat pixfmt)
	: RawFrameTextureBase(pixfmt)
	, m_rgb32TexRes(NULL)
	//, m_rgbTex(NULL), m_rgbSRV(NULL)
{
	//ARGBTexture_8类型的Texture只支持ARGB或者RGB内存布局的图片显示
	if(pixfmt!=PIXFMT_A8R8G8B8 && pixfmt!=PIXFMT_R8G8B8 
		&& pixfmt!=PIXFMT_B8G8R8A8 && pixfmt!=PIXFMT_R8G8B8A8 
		&& pixfmt!=PIXFMT_X8R8G8B8 && pixfmt!=PIXFMT_B8G8R8X8)
		m_pixfmt = PIXFMT_UNKNOW;
}

ARGBTexture_8::~ARGBTexture_8()
{
	destroy();
}


int ARGBTexture_8::create(ID3D11Device* device, int width, int height,
								const char* initData, int dataLen)
{
	if(PIXFMT_UNKNOW==m_pixfmt)
	{
#ifdef _DEBUG
		printf("Error in ARGBTexture_8::create : pixel format is Not support.(PIXFMT=%d)\n", m_pixfmt);
#endif
		return -1;
	}
	if(device==NULL || width<=0 || height<=0)
		return -1;
	
	m_VideoFrame = create_txframe(device, width, height, initData, dataLen);
	if(!m_VideoFrame.valid())
		return -3;
	m_device = device;
	m_width = width;
	m_height = height;
	return 0;
}

int ARGBTexture_8::create(ID3D11Device * device, int width, int height, TEXTURE_USAGE usage, bool bShared, const char * initData, int dataLen, int pitch)
{
	if (NULL == device || 0 >= width || 0 >= height || (TEXTURE_USAGE_DEFAULT != usage && bShared))
		return -1;
	if (PIXFMT_UNKNOW == m_pixfmt)
	{
#ifdef _DEBUG
		printf("Error in ARGBTexture_8::create : pixel format is Not support.(PIXFMT=%d)\n", m_pixfmt);
#endif
		return -2;
	}
	DXGI_FORMAT dstPixfmt = DXGI_FORMAT_UNKNOWN;
	switch (m_pixfmt)
	{
	case PIXFMT_A8R8G8B8:
	case PIXFMT_R8G8B8A8:
		dstPixfmt = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case PIXFMT_R8G8B8:
		dstPixfmt = DXGI_FORMAT_B8G8R8A8_UNORM;
		break;
	case PIXFMT_B8G8R8A8:
		dstPixfmt = DXGI_FORMAT_B8G8R8A8_UNORM;
		break;
	case PIXFMT_B8G8R8X8:
		dstPixfmt = DXGI_FORMAT_B8G8R8X8_UNORM;
		break;
	case PIXFMT_X8R8G8B8:
		dstPixfmt = DXGI_FORMAT_B8G8R8X8_UNORM;
		break;
	default:
		return -3;
	}
	TextureResource* rgb32Tex = new TextureResource();
	if (0 != rgb32Tex->create(device, width, height, dstPixfmt, usage, bShared, initData, dataLen, pitch))
	{
		delete rgb32Tex;
		return -4;
	}
	m_rgb32TexRes = rgb32Tex;
	if (m_textureCount >= 1)
	{
		m_textureArray[0] = m_rgb32TexRes;
	}
	m_device = device;
	m_width = width;
	m_height = height;
	return 0;
}

int ARGBTexture_8::destroy()
{
	if (m_rgb32TexRes)
	{
		m_textureArray[0] = NULL;
		m_rgb32TexRes->release();
		m_rgb32TexRes = NULL;
	}
	m_VideoFrame.destroy();
	m_device = NULL;
	m_width = 0;
	m_height = 0;
	return 0;
}

int ARGBTexture_8::getTexture(ID3D11Texture2D** outYUVTexs, int& texsCount) const
{
	return m_VideoFrame.getTexture(outYUVTexs, texsCount);
}

int ARGBTexture_8::getShaderResourceView(ID3D11ShaderResourceView** outYUVSRVs, int& srvsCount) const
{
	if (m_VideoFrame.valid())
	{
		return m_VideoFrame.getShaderResourceView(outYUVSRVs, srvsCount);
	}
	else
	{
		ID3D11ShaderResourceView* srv = m_textureArray[0]->getResourceView();
		if (srv == NULL)
		{
			if (0 == m_textureArray[0]->createResourceView())
			{
				srv = m_textureArray[0]->getResourceView();
			}
			else
			{
				return -2;
			}
		}
		outYUVSRVs[0] = srv;
		srvsCount = 1;
		return 0;
	}
}

int ARGBTexture_8::update(const unsigned char* pData, int dataLen, int yPitch, int uPitch, int vPitch, int width, int height,
							const RECT& regionUpdated, ID3D11DeviceContext* d3dDevContex)
{
	if(PIXFMT_UNKNOW==m_pixfmt || (!m_VideoFrame.valid() && m_textureArray[0] == NULL))
	{
#ifdef _DEBUG
		printf("Error in ARGBTexture_8::update : Texture Not inite yet.(PIXFMT=%d Tex=%d SRV=%d)\n", 
			m_pixfmt, (int)m_VideoFrame.m_rgbTex, (int)m_VideoFrame.m_rgbSRV);
#endif
		return -1;
	}
	if(width<=0 || height<=0 || pData==NULL || yPitch<FRAMEPITCH(width, m_pixfmt)
		|| dataLen < yPitch*height || (m_VideoFrame.valid() && d3dDevContex==NULL))
	{
#ifdef _DEBUG
		printf("Error in ARGBTexture_8::update : param invalid.(Data=%d dataLen=%d Pitch=%d width=%d height=%d Ctx=%d)\n",
			pData, dataLen, yPitch, width, height, (int)d3dDevContex);
#endif
		return -2;
	}
	if (m_VideoFrame.valid())
	{
		switch (m_pixfmt)
		{
		case PIXFMT_A8R8G8B8:
		case PIXFMT_R8G8B8A8:
		case PIXFMT_B8G8R8A8:
		case PIXFMT_X8R8G8B8:
		case PIXFMT_B8G8R8X8:
			return m_VideoFrame.update_A8R8G8B8(pData, dataLen, yPitch, width, height, regionUpdated, d3dDevContex);
		case PIXFMT_R8G8B8:
			return m_VideoFrame.update_R8G8B8(pData, dataLen, yPitch, width, height, regionUpdated, d3dDevContex);
		default:
			return -8;
		}
	}
	else if(m_textureArray[0]!=NULL)
	{
		return m_rgb32TexRes->update(pData, dataLen, yPitch, width, height, regionUpdated);
	}
	return -10;
//	if(m_pixfmt==PIXFMT_A8R8G8B8)
//		return m_VideoFrame.update_A8R8G8B8(pData, dataLen, yPitch, width, height, regionUpdated, d3dDevContex);
//	else if(m_pixfmt==PIXFMT_R8G8B8)
//		return m_VideoFrame.update_R8G8B8(pData, dataLen, yPitch, width, height, regionUpdated, d3dDevContex);
//	else return -8;
}

int zRender::ARGBTexture_8::FrameTexture::update_A8R8G8B8( const unsigned char* pData, int dataLen, int dataPitch, int width, int height, const RECT& regionUpdated, ID3D11DeviceContext* d3dDevContex )
{
	int regWidth = regionUpdated.right - regionUpdated.left;
	int regHeight = regionUpdated.bottom - regionUpdated.top;
	if(regionUpdated.left>=width || regionUpdated.top>=height || regionUpdated.left<0 || regionUpdated.top<0
		|| regionUpdated.right>width || regionUpdated.bottom>height || regWidth<=0 || regHeight<=0)
	{
#ifdef _DEBUG
		printf("Error in ARGBTexture_8::update : Update region is invalid.(L=%d R=%d T=%d B=%d)\n",
			regionUpdated.left, regionUpdated.right, regionUpdated.top, regionUpdated.bottom);
#endif
		return -3;
	}

	int updatedWidth = regWidth >= m_width ? m_width : regWidth;
	int updatedHeight = regHeight >= m_height ? m_height : regHeight;
	int startPosHrz = regionUpdated.left;
	int startPosVtc = regionUpdated.top;
	int dataLenCopyed = (int)(updatedWidth * PixelByteCount[m_pixfmt]);
	unsigned char* pDataStartPos = (unsigned char*)pData + (startPosVtc * dataPitch + (int)(startPosHrz * PixelByteCount[m_pixfmt]));

	D3D11_MAPPED_SUBRESOURCE mappedRes;
	ZeroMemory(&mappedRes, sizeof(mappedRes));
	D3D11_TEXTURE2D_DESC texDesc;
	m_rgbTexStage->GetDesc(&texDesc);
	HRESULT rslt = S_OK;
	if(S_OK!=(rslt=d3dDevContex->Map(m_rgbTexStage, 0, D3D11_MAP_WRITE, /*D3D11_MAP_FLAG_DO_NOT_WAIT*/0, &mappedRes)))
	{
#ifdef _DEBUG
		printf("Error in Error in ARGBTexture_8::update : ID3D11DeviceContext obj is Not match to this Texture.\n");
#endif
		TCHAR errmsg[1024] = {0};
		swprintf_s(errmsg, 1024, L"YUVTexture_Packed::update : ID3D11DeviceContext obj is Not match to this BGRA-Texture.this=[%d] result=[%d] errmsg=[%s]\n",
			(unsigned int)this, rslt, L"");
		OutputDebugString(errmsg);
		log_e(LOG_TAG, errmsg);
		return -4;
	}
	unsigned char* pDes = (unsigned char*)mappedRes.pData;
	for(int iVtc=0; iVtc<updatedHeight; iVtc++)
	{				
		memcpy(pDes, pDataStartPos, dataLenCopyed);
		pDes += mappedRes.RowPitch;
		pDataStartPos += dataPitch;
	}
	d3dDevContex->Unmap(m_rgbTexStage, 0);
	D3D11_BOX box;
	box.front = 0;
	box.back = 1;
	box.left = /*startPosHrz*/0;
	box.right = /*startPosHrz + */updatedWidth;
	box.top = /*startPosVtc*/0;
	box.bottom = /*startPosVtc + */updatedHeight;
	d3dDevContex->CopySubresourceRegion(m_rgbTex, 0, 0, 0, 0, m_rgbTexStage, 0, &box);
	return 0;
}

int zRender::ARGBTexture_8::FrameTexture::update_R8G8B8( const unsigned char* pData, int dataLen, int dataPitch, int width, int height, 
										   const RECT& regionUpdated, ID3D11DeviceContext* d3dDevContex )
{
	int regWidth = regionUpdated.right - regionUpdated.left;
	int regHeight = regionUpdated.bottom - regionUpdated.top;
	if(regionUpdated.left>=width || regionUpdated.top>=height || regionUpdated.left<0 || regionUpdated.top<0
		|| regionUpdated.right>width || regionUpdated.bottom>height || regWidth<=0 || regHeight<=0)
	{
#ifdef _DEBUG
		printf("Error in ARGBTexture_8::update : Update region is invalid.(L=%d R=%d T=%d B=%d)\n",
			regionUpdated.left, regionUpdated.right, regionUpdated.top, regionUpdated.bottom);
#endif
		return -3;
	}

	int updatedWidth = regWidth >= m_width ? m_width : regWidth;
	int updatedHeight = regHeight >= m_height ? m_height : regHeight;
	int startPosHrz = regionUpdated.left;
	int startPosVtc = regionUpdated.top;
	int dataLenCopyed = (int)(updatedWidth * PixelByteCount[m_pixfmt]);
	unsigned char* pDataStartPos = (unsigned char*)pData + (startPosVtc * dataPitch + (int)(startPosHrz * PixelByteCount[m_pixfmt]));

	D3D11_MAPPED_SUBRESOURCE mappedRes;
	ZeroMemory(&mappedRes, sizeof(mappedRes));
	D3D11_TEXTURE2D_DESC texDesc;
	m_rgbTexStage->GetDesc(&texDesc);
	HRESULT rslt = S_OK;
	if(S_OK!=(rslt=d3dDevContex->Map(m_rgbTexStage, 0, D3D11_MAP_WRITE, D3D11_MAP_FLAG_DO_NOT_WAIT, &mappedRes)))
	{
#ifdef _DEBUG
		printf("Error in Error in ARGBTexture_8::update : ID3D11DeviceContext obj is Not match to this Texture.\n");
#endif
		TCHAR errmsg[1024] = {0};
		swprintf_s(errmsg, 1024, L"YUVTexture_Packed::update : ID3D11DeviceContext obj is Not match to this RGB-Texture.this=[%d] result=[%d] errmsg=[%s]\n",
			(unsigned int)this, rslt, L"");
		OutputDebugString(errmsg);
		log_e(LOG_TAG, errmsg);
		return -4;
	}
	unsigned char* pDes = (unsigned char*)mappedRes.pData;
	for(int iVtc=0; iVtc<updatedHeight; iVtc++)
	{		
		unsigned char* pLine = pDes;
		unsigned char* pSrcLine = pDataStartPos;
		for (int iHrz=0; iHrz<updatedWidth; iHrz++)
		{
			pLine[0] = pSrcLine[0];
			pLine[1] = pSrcLine[1];
			pLine[2] = pSrcLine[2];
			pLine[3] = 0xff;
			pSrcLine += 3;
			pLine += 4;
		}

		pDes += mappedRes.RowPitch;
		pDataStartPos += dataPitch;
	}
	d3dDevContex->Unmap(m_rgbTexStage, 0);
	D3D11_BOX box;
	box.front = 0;
	box.back = 1;
	box.left = /*startPosHrz*/0;
	box.right = /*startPosHrz + */updatedWidth;
	box.top = /*startPosVtc*/0;
	box.bottom = /*startPosVtc + */updatedHeight;
	d3dDevContex->CopySubresourceRegion(m_rgbTex, 0, 0, 0, 0, m_rgbTexStage, 0, &box);
	return 0;
}

zRender::ARGBTexture_8::FrameTexture zRender::ARGBTexture_8::create_txframe( ID3D11Device* device, int width, int height, const char* initData, int dataLen )
{
	/*D3D11_TEXTURE2D_DESC texDesc;
	texDesc.ArraySize = texDesc.MipLevels = 1;
	texDesc.Height = height;
	texDesc.Width = width;
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
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
		initSubData.SysMemPitch = (UINT)FRAMEPITCH(width, m_pixfmt);
		pSubResData = &initSubData;
	}

	if(FAILED(device->CreateTexture2D(&texDesc, pSubResData, &m_rgbTex)))
		goto ErrorEnd;*/
	FrameTexture ft;
	int frameSize = (int)(FRAMESIZE(width, height, m_pixfmt));
	if(frameSize<=0)
		return ft;
	DXGI_FORMAT dstPixfmt = DXGI_FORMAT_UNKNOWN;
	switch(m_pixfmt)
	{
 	case PIXFMT_A8R8G8B8:
	case PIXFMT_R8G8B8A8:
		dstPixfmt = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case PIXFMT_R8G8B8:
		dstPixfmt = DXGI_FORMAT_B8G8R8A8_UNORM;
		break;
	case PIXFMT_B8G8R8A8:
		dstPixfmt = DXGI_FORMAT_B8G8R8A8_UNORM;
		break;
	case PIXFMT_B8G8R8X8:
		dstPixfmt = DXGI_FORMAT_B8G8R8X8_UNORM;
		break;
	case PIXFMT_X8R8G8B8:
		dstPixfmt = DXGI_FORMAT_B8G8R8X8_UNORM;
		break;
	default:
		return ft;
 	}
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.ArraySize = texDesc.MipLevels = 1;
	texDesc.Height = height;
	texDesc.Width = width;
	texDesc.Format = dstPixfmt;//DXGI_FORMAT_B8G8R8A8_UNORM;
	texDesc.SampleDesc.Quality = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = /*D3D11_CPU_ACCESS_WRITE*/0;
	texDesc.MiscFlags = 0;
	texDesc.Usage = /*D3D11_USAGE_DYNAMIC*/D3D11_USAGE_DEFAULT;

	D3D11_TEXTURE2D_DESC texDescStage;
	texDescStage.ArraySize = texDescStage.MipLevels = 1;
	texDescStage.Height = height;
	texDescStage.Width = width;
	texDescStage.Format = dstPixfmt;//DXGI_FORMAT_B8G8R8A8_UNORM;
	texDescStage.SampleDesc.Quality = 0;
	texDescStage.SampleDesc.Count = 1;
	texDescStage.BindFlags = 0;
	texDescStage.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	texDescStage.MiscFlags = 0;
	texDescStage.Usage = D3D11_USAGE_STAGING;

	D3D11_SUBRESOURCE_DATA* pSubResData = NULL;
	D3D11_SUBRESOURCE_DATA initSubData;
	ZeroMemory(&initSubData, sizeof(D3D11_SUBRESOURCE_DATA));
	if(initData && dataLen>=frameSize)
	{
		initSubData.pSysMem = initData;
		initSubData.SysMemPitch = width;
		pSubResData = &initSubData;
	}
	HRESULT rslt = S_OK;
	if(FAILED(rslt = device->CreateTexture2D(&texDescStage, pSubResData, &ft.m_rgbTexStage)))
		goto ErrorEnd;
	if(FAILED(device->CreateTexture2D(&texDesc, pSubResData, &ft.m_rgbTex)))
		goto ErrorEnd;	

	D3D11_SHADER_RESOURCE_VIEW_DESC	srvDesc;
	ZeroMemory(&srvDesc,sizeof(srvDesc));
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	if(FAILED(device->CreateShaderResourceView(ft.m_rgbTex, &srvDesc, &ft.m_rgbSRV)))
		goto ErrorEnd;

	ft.m_width = width;
	ft.m_height = height;
	ft.m_pixfmt = m_pixfmt;

	m_device = device;
	m_width = width;
	m_height = height;
	return ft;
ErrorEnd:
	destroy();
	return ft;
}

int zRender::ARGBTexture_8::update(SharedTexture* pSharedTexture, const RECT& regionUpdated, ID3D11DeviceContext* d3dDevContex)
{
	return -1;
}

bool zRender::ARGBTexture_8::FrameTexture::valid() const
{
	return m_rgbTex!=NULL && m_rgbSRV!=NULL;
}

void zRender::ARGBTexture_8::FrameTexture::destroy()
{
	ReleaseCOM(m_rgbSRV);
	ReleaseCOM(m_rgbTex);
	ReleaseCOM(m_rgbTexStage);
	m_width = 0;
	m_height = 0;
}

int zRender::ARGBTexture_8::FrameTexture::getTexture( ID3D11Texture2D** outYUVTexs, int& texsCount ) const
{
	if(texsCount<1 || outYUVTexs==NULL)
		return -1;
	if(m_rgbTex==NULL || m_rgbSRV==NULL)
		return -2;

	*(outYUVTexs) = m_rgbTex;
	texsCount = 1;

	return 0;
}

int zRender::ARGBTexture_8::FrameTexture::getShaderResourceView( ID3D11ShaderResourceView** outYUVSRVs, int& srvsCount ) const
{
	if(srvsCount<1 || outYUVSRVs==NULL)
		return -1;
	if(m_rgbTex==NULL || m_rgbSRV==NULL)
		return -2;

	*(outYUVSRVs+0) = m_rgbSRV;
	srvsCount = 1;

	return 0;
}
