#include "YUVTexture_Packed.h"
#include "DxRenderCommon.h"
#include "DXLogger.h"
#include "inc/TextureResource.h"

using namespace zRender;

#define LOG_TAG L"D3D11_YUVTexture_Packed"

YUVTexture_Packed::YUVTexture_Packed(PIXFormat pixfmt)
	: RawFrameTextureBase(pixfmt)
	, m_yuvTexRes(NULL), m_parityTexRes(NULL)
{
	//YUVTexture_Packed类型的Texture只支持YUY2这样紧凑型的YUV内存布局的图片显示
	if(pixfmt!=PIXFMT_YUY2)
		m_pixfmt = PIXFMT_UNKNOW;
}

YUVTexture_Packed::~YUVTexture_Packed()
{
	destroy();
}


int YUVTexture_Packed::create(ID3D11Device* device, int width, int height,
								const char* initData, int dataLen)
{
	if(PIXFMT_UNKNOW==m_pixfmt)
	{
#ifdef _DEBUG
		printf("Error in YUVTexture_Packed::create : pixel format is Not support.(PIXFMT=%d)\n", m_pixfmt);
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

int zRender::YUVTexture_Packed::create(ID3D11Device * device, int width, int height, TEXTURE_USAGE usage, bool bShared, const char * initData, int dataLen, int pitch)
{
	if (NULL == device || 0 >= width || 0 >= height || (TEXTURE_USAGE_DEFAULT != usage && bShared))
		return -1;
	if (PIXFMT_UNKNOW == m_pixfmt)
	{
#ifdef _DEBUG
		printf("Error in YUVTexture_Packed::create : pixel format is Not support.(PIXFMT=%d)\n", m_pixfmt);
#endif
		return -2;
	}
	TextureResource* yuvTex = new TextureResource();
	if (0 != yuvTex->create(device, width, height, DXGI_FORMAT_R8G8_UNORM, usage, bShared, initData, dataLen, pitch))
	{
		delete yuvTex;
		return -3;
	}
	m_yuvTexRes = yuvTex;
	TextureResource* parityTexRes = new TextureResource();
	unsigned char* parityBuf = (unsigned char*)malloc(width*height);
	for (int indexHeight = 0; indexHeight < height; indexHeight++)
	{
		for (int indexWidth = 0; indexWidth < width; indexWidth++)
		{
			parityBuf[width*indexHeight + indexWidth] = (indexWidth % 2) * 255;
		}
	}
	if (0 != parityTexRes->create(device, width, height, DXGI_FORMAT_R8_UNORM, usage, bShared, (char*)parityBuf, width*height, width))
	{
		free(parityBuf);
		delete parityTexRes;
		return -4;
	}
	free(parityBuf);
	m_parityTexRes = parityTexRes;
	if (m_textureCount >= 2)
	{
		m_textureArray[0] = m_yuvTexRes;
		m_textureArray[1] = m_parityTexRes;
	}
	m_device = device;
	m_width = width;
	m_height = height;
	return 0;
}

int YUVTexture_Packed::destroy()
{
	if (m_yuvTexRes)
	{
		m_yuvTexRes->release();
		m_yuvTexRes = NULL;
	}
	if (m_parityTexRes)
	{
		m_parityTexRes->release();
		m_parityTexRes = NULL;
	}
	m_VideoFrame.destroy();
	m_device = NULL;
	m_width = 0;
	m_height = 0;
	return 0;
}

int YUVTexture_Packed::getTexture(ID3D11Texture2D** outYUVTexs, int& texsCount) const
{
	if(texsCount<2 || outYUVTexs==NULL)
		return -1;

	return m_VideoFrame.getTexture(outYUVTexs, texsCount);
}

int YUVTexture_Packed::getShaderResourceView(ID3D11ShaderResourceView** outYUVSRVs, int& srvsCount) const
{
	if(srvsCount<2 || outYUVSRVs==NULL)
		return -1;

	if (m_VideoFrame.valid())
	{
		return m_VideoFrame.getShaderResourceView(outYUVSRVs, srvsCount);
	}
	//else
	//{
	//	ID3D11ShaderResourceView* srv = m_textureArray[0]->getResourceView();
	//	if (srv == NULL)
	//	{
	//		if (0 == m_textureArray[0]->createResourceView())
	//		{
	//			srv = m_textureArray[0]->getResourceView();
	//		}
	//		else
	//		{
	//			return -2;
	//		}
	//	}
	//	outYUVSRVs[0] = srv;
	//	srv = m_textureArray[1]->getResourceView();
	//	if (srv == NULL)
	//	{
	//		if (0 == m_textureArray[1]->createResourceView())
	//		{
	//			srv = m_textureArray[1]->getResourceView();
	//		}
	//		else
	//		{
	//			return -3;
	//		}
	//	}
	//	outYUVSRVs[1] = srv;
	//	srvsCount = 2;
	//	return 0;
	//}
}

int YUVTexture_Packed::update(const unsigned char* pData, int dataLen, int yPitch, int uPitch, int vPitch, int width, int height,
							const RECT& regionUpdated, ID3D11DeviceContext* d3dDevContex)
{
	if(PIXFMT_UNKNOW==m_pixfmt)
	{
		return -1;
	}
	if (m_VideoFrame.valid())
	{
		FrameTexture& ft = m_VideoFrame;
		int ret = ft.update(pData, dataLen, yPitch, uPitch, vPitch, width, height, regionUpdated, d3dDevContex);
		return ret;
	}
	//else if (m_textureArray[0] && m_textureArray[1])
	//{
	//	m_textureArray[0]->update(pData, dataLen, yPitch, width, height, regionUpdated);
	//}
}

int zRender::YUVTexture_Packed::update(SharedTexture* pSharedTexture, const RECT& regionUpdated, ID3D11DeviceContext* d3dDevContex)
{
	return -1;
}

int zRender::YUVTexture_Packed::FrameTexture::update( const unsigned char* pData, int dataLen, int yPitch, int uPitch, int vPitch, int width, int height, const RECT& regionUpdated, ID3D11DeviceContext* d3dDevContex )
{
	if(PIXFMT_UNKNOW==m_pixfmt || m_yuvTex==NULL || m_yuvSRV==NULL)
	{
#ifdef _DEBUG
		printf("Error in YUVTexture_Packed::update : Texture Not inite yet.(PIXFMT=%d Tex=%d SRV=%d)\n", 
			m_pixfmt, (int)m_yuvTex, (int)m_yuvSRV);
#endif
		return -1;
	}
	if(width<=0 || height<=0 || pData==NULL || yPitch<FRAMEPITCH(width, m_pixfmt)
		|| dataLen < yPitch*height || d3dDevContex==NULL)
	{
#ifdef _DEBUG
		printf("Error in YUVTexture_Packed::update : param invalid.(Data=%d dataLen=%d Pitch=%d width=%d height=%d Ctx=%d)\n",
			pData, dataLen, yPitch, width, height, (int)d3dDevContex);
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
		return -3;
#endif
	}

	int updatedWidth = regWidth >= m_width ? m_width : regWidth;
	int updatedHeight = regHeight >= m_height ? m_height : regHeight;
	int startPosHrz = regionUpdated.left;
	int startPosVtc = regionUpdated.top;
	int dataLenCopyed = (int)(updatedWidth * PixelByteCount[m_pixfmt]);
	unsigned char* pDataStartPos = (unsigned char*)pData + (startPosVtc * yPitch + (int)(startPosHrz * PixelByteCount[m_pixfmt]));

	D3D11_MAPPED_SUBRESOURCE mappedRes;
	ZeroMemory(&mappedRes, sizeof(mappedRes));
	D3D11_TEXTURE2D_DESC texDesc;
	m_yuvTexStage->GetDesc(&texDesc);
	HRESULT rslt = S_OK;
	if(S_OK!=(rslt=d3dDevContex->Map(m_yuvTexStage, 0, D3D11_MAP_WRITE, D3D11_MAP_FLAG_DO_NOT_WAIT, &mappedRes)))
	{
#ifdef _DEBUG
		printf("Error in Error in YUVTexture_Packed::update : ID3D11DeviceContext obj is Not match to this Texture.\n");
#endif
		TCHAR errmsg[1024] = {0};
		swprintf_s(errmsg, 1024, L"YUVTexture_Packed::update : ID3D11DeviceContext obj is Not match to this YUV-Texture.this=[%d] result=[%d] errmsg=[%s]\n",
			(unsigned int)this, rslt, L"");
		OutputDebugString(errmsg);
		log_e(LOG_TAG, errmsg, 20);
		return -4;
	}
	unsigned char* pDes = (unsigned char*)mappedRes.pData;
	for(int iVtc=0; iVtc<updatedHeight; iVtc++)
	{				
		//int dataPos = iVtc * dataPitch + startPosHrz * PixelByteCount[m_pixfmt];
		//unsigned char* pVtcData = (unsigned char*)pData + dataPos;
		memcpy(pDes, pDataStartPos, dataLenCopyed);
		pDes += mappedRes.RowPitch;
		pDataStartPos += yPitch;
	}
	d3dDevContex->Unmap(m_yuvTexStage, 0);
	D3D11_BOX box;
	box.front = 0;
	box.back = 1;
	box.left = /*startPosHrz*/0;
	box.right = /*startPosHrz + */updatedWidth;
	box.top = /*startPosVtc*/0;
	box.bottom = /*startPosVtc + */updatedHeight;
	d3dDevContex->CopySubresourceRegion(m_yuvTex, 0, 0, 0, 0, m_yuvTexStage, 0, &box);
	return 0;
}

int zRender::YUVTexture_Packed::FrameTexture::getTexture( ID3D11Texture2D** outYUVTexs, int& texsCount ) const
{
	if(m_yuvTex==NULL || m_yuvSRV==NULL || m_paritySRV==NULL || NULL==m_parityTex)
		return -2;

	*(outYUVTexs) = m_yuvTex;
	*(outYUVTexs+1) = m_parityTex;
	texsCount = 2;
	return 0;
}

int zRender::YUVTexture_Packed::FrameTexture::getShaderResourceView( ID3D11ShaderResourceView** outYUVSRVs, int& srvsCount ) const
{
	if(m_yuvTex==NULL || m_yuvSRV==NULL|| m_paritySRV==NULL || NULL==m_parityTex)
		return -2;

	*(outYUVSRVs+0) = m_yuvSRV;
	*(outYUVSRVs+1) = m_paritySRV;
	srvsCount = 2;
	return 0;
}

void zRender::YUVTexture_Packed::FrameTexture::destroy()
{
	ReleaseCOM(m_yuvSRV);
	ReleaseCOM(m_paritySRV);
	ReleaseCOM(m_yuvTex);
	ReleaseCOM(m_parityTex);
	ReleaseCOM(m_yuvTexStage);
	m_width = 0;
	m_height = 0;
	m_pixfmt = zRender::PIXFMT_UNKNOW;
}

bool zRender::YUVTexture_Packed::FrameTexture::valid() const
{
	return m_yuvTex!=NULL && m_parityTex!=NULL && m_yuvSRV!=NULL && m_paritySRV!=NULL && m_yuvTexStage!=NULL;
}

zRender::YUVTexture_Packed::FrameTexture YUVTexture_Packed::create_txframe(ID3D11Device* device, int width, int height, const char* initData, int dataLen)
{
	YUVTexture_Packed::FrameTexture ft;
	if(PIXFMT_UNKNOW==m_pixfmt)
	{
#ifdef _DEBUG
		printf("Error in YUVTexture_Packed::create : pixel format is Not support.(PIXFMT=%d)\n", m_pixfmt);
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
	texDesc.Format = DXGI_FORMAT_R8G8_UNORM;
	texDesc.SampleDesc.Quality = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0/*D3D11_CPU_ACCESS_WRITE*/;
	texDesc.MiscFlags = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_TEXTURE2D_DESC texDescStage;
	texDescStage.ArraySize = texDescStage.MipLevels = 1;
	texDescStage.Height = height;
	texDescStage.Width = width;
	texDescStage.Format = DXGI_FORMAT_R8G8_UNORM;
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
		initSubData.SysMemPitch = dataLen / height;//(int)FRAMEPITCH(width, m_pixfmt);
		pSubResData = &initSubData;
	}

	HRESULT rslt = S_OK;
	if(FAILED(rslt = device->CreateTexture2D(&texDescStage, pSubResData, &ft.m_yuvTexStage)))
		goto ErrorEnd;
	if(FAILED(rslt = device->CreateTexture2D(&texDesc, pSubResData, &ft.m_yuvTex)))
		goto ErrorEnd;
	

	D3D11_SHADER_RESOURCE_VIEW_DESC	srvDesc;
	ZeroMemory(&srvDesc,sizeof(srvDesc));
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	if(FAILED(device->CreateShaderResourceView(ft.m_yuvTex, &srvDesc, &ft.m_yuvSRV)))
		goto ErrorEnd;

	texDesc.Format = DXGI_FORMAT_R8_UNORM;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.CPUAccessFlags = 0;
	unsigned char* parityBuf = (unsigned char*)malloc(width*height);
	for (int indexHeight = 0; indexHeight < height; indexHeight++)
	{
		for (int indexWidth = 0; indexWidth < width; indexWidth++)
		{
			parityBuf[width*indexHeight + indexWidth] = (indexWidth % 2) * 255;
		}
	}
	initSubData.pSysMem = parityBuf;
	initSubData.SysMemPitch = width;
	pSubResData = &initSubData;
	if(FAILED(device->CreateTexture2D(&texDesc, pSubResData, &ft.m_parityTex)))
	{
		free(parityBuf);
		goto ErrorEnd;
	}
	free(parityBuf);

	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	if(FAILED(device->CreateShaderResourceView(ft.m_parityTex, &srvDesc, &ft.m_paritySRV)))
		goto ErrorEnd;

	m_device = device;
	m_width = width;
	m_height = height;

	ft.m_width = width;
	ft.m_height = height;
	ft.m_pixfmt = m_pixfmt;
	return ft;
ErrorEnd:
	destroy();
	return YUVTexture_Packed::FrameTexture();
}
