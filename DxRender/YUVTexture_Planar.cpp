#pragma warning (disable:4995)

#include "YUVTexture_Planar.h"
#include <fstream>
#include <assert.h>
#include <Windows.h>
#include "DXLogger.h"

using namespace std;
using namespace zRender;

const float factorToFrameBufLen_yuvPlanar = 1.5;

#define FrameSize_YUVPlanar(x, y) ((x)*(y)*(factorToFrameBufLen_yuvPlanar))
#define LOG_TAG L"D3D11_YUVTexture_Plannar"

YUVTexture_Planar::YUVTexture_Planar(PIXFormat yuvFmt)
	: RawFrameTextureBase(yuvFmt)
{
	if(m_pixfmt!=PIXFMT_YUV420P && m_pixfmt!=PIXFMT_YV12)
		m_pixfmt = PIXFMT_UNKNOW;
}

YUVTexture_Planar::~YUVTexture_Planar()
{
	destroy();
}

int YUVTexture_Planar::create(ID3D11Device* device, int width, int height, const char* initData, int dataLen)
{
	if(PIXFMT_UNKNOW==m_pixfmt)
	{
#ifdef _DEBUG
		printf("Error in YUVTexture_Planar::create : pixel format is Not support.(PIXFMT=%d)\n", m_pixfmt);
#endif
		return -1;
	}
	if(device==NULL || width<=0 || height<=0)
		return -1;
	int frameSize = (int)FRAMESIZE(width, height, m_pixfmt);
	if(frameSize<=0)
		return -2;
	
	m_VideoFrame = create_txframe(device, width, height, initData, dataLen);
	if(!m_VideoFrame.valid())
		return -3;
	m_device = device;
	m_width = width;
	m_height = height;
	return 0;
}

int YUVTexture_Planar::create(ID3D11Device * device, int width, int height, TEXTURE_USAGE usage, bool bShared, const char * initData, int dataLen, int pitch)
{
	return -1;
}

int YUVTexture_Planar::destroy()
{
	m_VideoFrame.destroy();
	m_device = NULL;
	m_width = 0;
	m_height = 0;
	return 0;
}

int YUVTexture_Planar::getTexture(ID3D11Texture2D** outYUVTexs, int& texsCount) const
{
	if(texsCount<3 || outYUVTexs==NULL)
		return -1;

	return m_VideoFrame.getTexture(outYUVTexs, texsCount);
// 	const FrameTexture& ft = m_VideoFrame;
// 	if(ft.m_yTex==NULL || NULL==ft.m_uTex || NULL==ft.m_vTex || NULL==ft.m_ySRV || NULL==ft.m_uSRV || NULL==ft.m_vSRV )
// 		return -2;
// 
// 	*(outYUVTexs+0) = ft.m_yTex;
// 	*(outYUVTexs+1) = ft.m_uTex;
// 	*(outYUVTexs+2) = ft.m_vTex;
// 	texsCount = 3;
// 	return 0;
}

int YUVTexture_Planar::getShaderResourceView(ID3D11ShaderResourceView** outYUVSRVs, int& srvsCount) const
{
	if(srvsCount<3 || outYUVSRVs==NULL)
		return -1;

	return m_VideoFrame.getShaderResourceView(outYUVSRVs, srvsCount);
// 	const FrameTexture& ft = m_VideoFrame;
// 	if(ft.m_yTex==NULL || NULL==ft.m_uTex || NULL==ft.m_vTex || NULL==ft.m_ySRV || NULL==ft.m_uSRV || NULL==ft.m_vSRV )
// 		return -2;
// 
// 	*(outYUVSRVs+0) = ft.m_ySRV;
// 	*(outYUVSRVs+1) = ft.m_uSRV;
// 	*(outYUVSRVs+2) = ft.m_vSRV;
// 	srvsCount = 3;
// 	return 0;
}

int YUVTexture_Planar::update(const unsigned char* pData, int dataLen, int yPitch, int uPitch, int vPitch, int width, int height,
							const RECT& regionUpdated, ID3D11DeviceContext* d3dDevContex)
{
	if(PIXFMT_UNKNOW==m_pixfmt)
	{
		return -1;
	}
	FrameTexture& ft = m_VideoFrame;
	int ret = ft.update(pData, dataLen, yPitch, uPitch, vPitch, width, height, regionUpdated, d3dDevContex);
	return ret;
}

int zRender::YUVTexture_Planar::FrameTexture::update( const unsigned char* pData, int dataLen, int yPitch, int uPitch, int vPitch, int width, int height, const RECT& regionUpdated, ID3D11DeviceContext* d3dDevContex )
{
	if(m_yTex==NULL || NULL==m_uTex || NULL==m_vTex || NULL==m_ySRV || NULL==m_uSRV || NULL==m_vSRV)
	{
#ifdef _DEBUG
		printf("Error in YUVTexture_Planar::update : Texture Not inite yet.(Tex_y=%d Tex_u=%d Tex_v=%d SRV_y=%d SRV_u=%d SRV_v=%d)\n", 
			(int)m_yTex, (int)m_uTex, (int)m_vTex, (int)m_ySRV, (int)m_uSRV, (int)m_vSRV);
#endif
		return -1;
	}
	if(width<=0 || height<=0 || pData==NULL	|| dataLen < FRAMESIZE(width, height, m_pixfmt) || d3dDevContex==NULL)
	{
#ifdef _DEBUG
		printf("Error in YUVTexture_Packed::update : param invalid.(Data=%d dataLen=%d width=%d height=%d Ctx=%d)\n",
			pData, dataLen, width, height, (int)d3dDevContex);
#endif
		return -2;
	}
	int regWidth = regionUpdated.right - regionUpdated.left;
	int regHeight = regionUpdated.bottom - regionUpdated.top;
	if(regionUpdated.left>=width || regionUpdated.top>=height || regionUpdated.left<0 || regionUpdated.top<0
		|| regionUpdated.right>width || regionUpdated.bottom>height || regWidth<=0 || regHeight<=0)
	{
#ifdef _DEBUG
		printf("Error in YUVTexture_Packed::update : Update region is invalid.(L=%d R=%d T=%d B=%d)\n",
			regionUpdated.left, regionUpdated.right, regionUpdated.top, regionUpdated.bottom);
#endif
		return -3;
	}

	int updatedWidth = regWidth >= m_width ? m_width : regWidth;
	int updatedHeight = regHeight >= m_height ? m_height : regHeight;
	int startPosHrz = regionUpdated.left;
	int startPosVtc = regionUpdated.top;
	unsigned char* startDataPos = NULL;

	D3D11_MAPPED_SUBRESOURCE mappedRes;
	ZeroMemory(&mappedRes, sizeof(mappedRes));
	D3D11_TEXTURE2D_DESC texDesc;
	m_yTexStage->GetDesc(&texDesc);
	HRESULT rslt = S_OK;
	if(S_OK!=(rslt=d3dDevContex->Map(m_yTexStage, 0, /*D3D11_MAP_WRITE_DISCARD*/D3D11_MAP_WRITE, D3D11_MAP_FLAG_DO_NOT_WAIT, &mappedRes)))
	{
#ifdef _DEBUG
		printf("Error in Error in YUVTexture_Planar::update : ID3D11DeviceContext obj is Not match to this Y-Texture.\n");
#endif
		TCHAR errmsg[1024] = {0};
		swprintf_s(errmsg, 1024, L"YUVTexture_Planar::update : ID3D11DeviceContext obj is Not match to this Y-Texture.this=[%d] result=[%d] errmsg=[%s]\n",
			(unsigned int)this, rslt, L"");
		OutputDebugString(errmsg);
		log_e(LOG_TAG, errmsg, 20);
		return -4;
	}
	startDataPos = (unsigned char*)pData + (startPosVtc * yPitch + startPosHrz);
	unsigned char* pDes = (unsigned char*)mappedRes.pData;
	for(int iVtc=0; iVtc<updatedHeight; iVtc++)
	{				
		memcpy(pDes, startDataPos, updatedWidth);
		pDes += mappedRes.RowPitch;
		startDataPos += yPitch;
	}
	d3dDevContex->Unmap(m_yTexStage, 0);
	D3D11_BOX box;
	box.front = 0;
	box.back = 1;
	box.left = 0;
	box.right = updatedWidth;
	box.top = 0;
	box.bottom = updatedHeight;
	d3dDevContex->CopySubresourceRegion(m_yTex, 0, 0, 0, 0, m_yTexStage, 0, &box);

	if(S_OK!=(rslt=d3dDevContex->Map(m_uTexStage, 0, /*D3D11_MAP_WRITE_DISCARD*/D3D11_MAP_WRITE, D3D11_MAP_FLAG_DO_NOT_WAIT, &mappedRes)))
	{
#ifdef _DEBUG
		printf("Error in Error in YUVTexture_Planar::update : ID3D11DeviceContext obj is Not match to this U-Texture.\n");
#endif
		TCHAR errmsg[1024] = {0};
		swprintf_s(errmsg, 1024, L"YUVTexture_Planar::update : ID3D11DeviceContext obj is Not match to this U-Texture.this=[%d] result=[%d] errmsg=[%s]\n",
			(unsigned int)this, rslt, L"");
		OutputDebugString(errmsg);
		log_e(LOG_TAG, errmsg, 20);
		return -5;
	}
	int startPosVtc_uv = (startPosVtc+1)/2;
	int startPosHrz_uv = (startPosHrz+1) / 2;
	int height_uv = (height+1)/2;
	int updatedHeight_uv = (updatedHeight+1)/2;
	int updatedWidth_uv = (updatedWidth+1) / 2;
	if(m_pixfmt==PIXFMT_YUV420P)
	{
		startDataPos = (unsigned char*)pData + ((yPitch * height) + ( (startPosVtc_uv) * uPitch + startPosHrz_uv) );
	}
	else
	{
		startDataPos = (unsigned char*)pData + ((yPitch * height) + vPitch * (height_uv) + ((startPosVtc_uv) * uPitch + startPosHrz_uv));
	}
	pDes = (unsigned char*)mappedRes.pData;
	for(int iVtc=0; iVtc<updatedHeight_uv; iVtc++)
	{				
		memcpy(pDes, startDataPos, updatedWidth_uv);
		pDes += mappedRes.RowPitch;
		startDataPos += uPitch;
	}
	d3dDevContex->Unmap(m_uTexStage, 0);
	box.front = 0;
	box.back = 1;
	box.left = 0;
	box.right = updatedWidth_uv;
	box.top = 0;
	box.bottom = updatedHeight_uv;
	d3dDevContex->CopySubresourceRegion(m_uTex, 0, 0, 0, 0, m_uTexStage, 0, &box);

	if(S_OK!=(rslt=d3dDevContex->Map(m_vTexStage, 0, D3D11_MAP_WRITE, D3D11_MAP_FLAG_DO_NOT_WAIT, &mappedRes)))
	{
#ifdef _DEBUG
		printf("Error in Error in YUVTexture_Planar::update : ID3D11DeviceContext obj is Not match to this V-Texture.\n");
#endif
		TCHAR errmsg[1024] = {0};
		swprintf_s(errmsg, 1024, L"YUVTexture_Planar::update : ID3D11DeviceContext obj is Not match to this Y-Texture.this=[%d] result=[%d] errmsg=[%s]\n",
			(unsigned int)this, rslt, L"");
		OutputDebugString(errmsg);
		log_e(LOG_TAG, errmsg, 20);
		return -6;
	}
	if(m_pixfmt==PIXFMT_YUV420P)
	{
		startDataPos = (unsigned char*)pData + ((yPitch * height) + uPitch * (height_uv) + ((startPosVtc_uv) * vPitch + startPosHrz_uv));
	}
	else
	{
		startDataPos = (unsigned char*)pData + ((yPitch * height) + ((startPosVtc_uv) * vPitch + startPosHrz_uv));
	}
	pDes = (unsigned char*)mappedRes.pData;
	for(int iVtc=0; iVtc<updatedHeight_uv; iVtc++)
	{				
		memcpy(pDes, startDataPos, updatedWidth_uv);
		pDes += mappedRes.RowPitch;
		startDataPos += vPitch;
		//assert((int)(startDataPos+updatedWidth/2) - (int)pData <= width*height*1.5);
	}
	d3dDevContex->Unmap(m_vTexStage, 0);
	box.front = 0;
	box.back = 1;
	box.left = 0;
	box.right = updatedWidth_uv;
	box.top = 0;
	box.bottom = updatedHeight_uv;
	d3dDevContex->CopySubresourceRegion(m_vTex, 0, 0, 0, 0, m_vTexStage, 0, &box);
	return 0;
}

int zRender::YUVTexture_Planar::FrameTexture::getTexture( ID3D11Texture2D** outYUVTexs, int& texsCount ) const
{
	if(m_yTex==NULL || NULL==m_uTex || NULL==m_vTex || NULL==m_ySRV || NULL==m_uSRV || NULL==m_vSRV )
		return -2;

	*(outYUVTexs+0) = m_yTex;
	*(outYUVTexs+1) = m_uTex;
	*(outYUVTexs+2) = m_vTex;
	texsCount = 3;
	return 0;
}

int zRender::YUVTexture_Planar::FrameTexture::getShaderResourceView( ID3D11ShaderResourceView** outYUVSRVs, int& srvsCount ) const
{
	if(m_yTex==NULL || NULL==m_uTex || NULL==m_vTex || NULL==m_ySRV || NULL==m_uSRV || NULL==m_vSRV )
		return -2;

	*(outYUVSRVs+0) = m_ySRV;
	*(outYUVSRVs+1) = m_uSRV;
	*(outYUVSRVs+2) = m_vSRV;
	srvsCount = 3;
	return 0;
}

void zRender::YUVTexture_Planar::FrameTexture::destroy()
{
	ReleaseCOM(m_ySRV);
	ReleaseCOM(m_uSRV);
	ReleaseCOM(m_vSRV);
	ReleaseCOM(m_yTex);
	ReleaseCOM(m_uTex);
	ReleaseCOM(m_vTex);
	ReleaseCOM(m_yTexStage);
	ReleaseCOM(m_uTexStage);
	ReleaseCOM(m_vTexStage);
	m_width = 0;
	m_height = 0;
	m_pixfmt = zRender::PIXFMT_UNKNOW;
}

bool zRender::YUVTexture_Planar::FrameTexture::valid() const
{
	return m_yTex!=NULL && m_uTex!=NULL && m_vTex!=NULL && m_ySRV!=NULL && m_uSRV!=NULL && m_vSRV!=NULL;
}

zRender::YUVTexture_Planar::FrameTexture YUVTexture_Planar::create_txframe(ID3D11Device* device, int width, int height, const char* initData, int dataLen)
{
	FrameTexture ft;
	if(PIXFMT_UNKNOW==m_pixfmt)
	{
#ifdef _DEBUG
		printf("Error in YUVTexture_Planar::create : pixel format is Not support.(PIXFMT=%d)\n", m_pixfmt);
#endif
		return ft;
	}
	if(device==NULL || width<=0 || height<=0)
		return ft;
	int frameSize = (int)FRAMESIZE(width, height, m_pixfmt);
	if(frameSize<=0)
		return ft;
	
		D3D11_TEXTURE2D_DESC texDesc;
		texDesc.ArraySize = texDesc.MipLevels = 1;
		texDesc.Height = height;
		texDesc.Width = width;
		texDesc.Format = DXGI_FORMAT_R8_UNORM;
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
		texDescStage.Format = DXGI_FORMAT_R8_UNORM;
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
		if(FAILED(rslt = device->CreateTexture2D(&texDescStage, pSubResData, &ft.m_yTexStage)))
			goto ErrorEnd;
		if(FAILED(device->CreateTexture2D(&texDesc, pSubResData, &ft.m_yTex)))
			goto ErrorEnd;	

		int uvTexWidth = (width+1) / 2;
		int uvTexHeight = (height+1) / 2;
		texDesc.Height = uvTexHeight;
		texDesc.Width = uvTexWidth;
		texDescStage.Height = uvTexHeight; 
		texDescStage.Width = uvTexWidth;
		ZeroMemory(&initSubData, sizeof(D3D11_SUBRESOURCE_DATA));
		if(initData && dataLen>=frameSize)
		{
			initSubData.pSysMem = initData + width * height;
			initSubData.SysMemPitch = uvTexWidth;
		}
		if(m_pixfmt==PIXFMT_YUV420P)
		{
			if(FAILED(device->CreateTexture2D(&texDesc, pSubResData, &ft.m_uTex)))
				goto ErrorEnd;
			if(FAILED(device->CreateTexture2D(&texDescStage, pSubResData, &ft.m_uTexStage)))
				goto ErrorEnd;
		}
		else
		{
			if(FAILED(device->CreateTexture2D(&texDesc, pSubResData, &ft.m_vTex)))
				goto ErrorEnd;
			if(FAILED(device->CreateTexture2D(&texDescStage, pSubResData, &ft.m_vTexStage)))
				goto ErrorEnd;
		}

		ZeroMemory(&initSubData, sizeof(D3D11_SUBRESOURCE_DATA));
		if(initData && dataLen>=frameSize)
		{
			initSubData.pSysMem = initData + width * height + (uvTexWidth)*(uvTexHeight);
			initSubData.SysMemPitch = uvTexWidth;
		}
		if(m_pixfmt==PIXFMT_YUV420P)
		{
			if(FAILED(device->CreateTexture2D(&texDesc, pSubResData, &ft.m_vTex)))
				goto ErrorEnd;
			if(FAILED(device->CreateTexture2D(&texDescStage, pSubResData, &ft.m_vTexStage)))
				goto ErrorEnd;
		}
		else
		{
			if(FAILED(device->CreateTexture2D(&texDesc, pSubResData, &ft.m_uTex)))
				goto ErrorEnd;
			if(FAILED(device->CreateTexture2D(&texDescStage, pSubResData, &ft.m_uTexStage)))
				goto ErrorEnd;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC	srvDesc;
		ZeroMemory(&srvDesc,sizeof(srvDesc));
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		if(FAILED(device->CreateShaderResourceView(ft.m_yTex, &srvDesc, &ft.m_ySRV)))
			goto ErrorEnd;

		if(FAILED(device->CreateShaderResourceView(ft.m_uTex, &srvDesc, &ft.m_uSRV)))
			goto ErrorEnd;

		if(FAILED(device->CreateShaderResourceView(ft.m_vTex, &srvDesc, &ft.m_vSRV)))
			goto ErrorEnd;
		ft.m_width = width;
		ft.m_height = height;
		ft.m_pixfmt = m_pixfmt;

	m_width = width;
	m_height = height;

	return ft;
ErrorEnd:
	destroy();
	return ft;
}
