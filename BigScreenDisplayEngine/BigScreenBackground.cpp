#include "BigScreenBackground.h"
//#include "turbojpeg.h"
#include <fstream>
#include <locale>
#include <stdio.h>
#include <Windows.h>
#include <string>
//#include "Decoder.h"
#include <assert.h>
//#include "VideoScaler.h"

using namespace SOA::Mirror::Render;
using namespace zRender;

BigScreenBackground::BigScreenBackground( const char* imageFileName, int bigscreenWidth, int bigscreenHeight, 
										  const Size& cellRsolution, zRender::PIXFormat pixfomat )
										  : m_decodedData(NULL)
										  , m_decodedDataLen(0)
										  , m_bigScreenWidth(bigscreenWidth)
										  , m_bigScreenHeight(bigscreenHeight)
										  , m_imageWidth(0)
										  , m_imageHeight(0)
										  , m_isSplited(false)
										  , m_pixformat(pixfomat)
										  , m_bigscreenCellResolution(cellRsolution)
{
	if(m_bigScreenHeight<=0 || m_bigScreenWidth<=0 || pixfomat==zRender::PIXFMT_UNKNOW)
	{
		return;
	}
	decodeBackgroundImageFile(imageFileName, pixfomat);
}

BigScreenBackground::~BigScreenBackground()
{
	freeBackgroundContent();
}

void BigScreenBackground::allocBackgroundContentForBigscreenCell()
{
	initBigScreenCellPaintRegion(m_bigscreenCellResolution);
	splitBackgroundImageForBigscreenCell();
}

void SOA::Mirror::Render::BigScreenBackground::freeBackgroundContent()
{
	if(m_decodedData!=NULL)
	{
		delete[] m_decodedData;
		m_decodedData = NULL;
		m_decodedDataLen = 0;
	}
	for(int i=0; i<m_regions.size(); i++)
	{
		IDisplayContentProvider* dcp = m_regions[i].m_dcproviders;
		if(dcp)
		{
			TextureDataSource* tds = dcp->getTextureDataSource();
			delete tds;
		}
		delete dcp;
		m_regions[i].m_dcproviders = NULL;
	}
}

const RectCoordinate& BigScreenBackground::getBigscreenCellPaintRegion( int XofBigScreen, int YofBigScreen )
{
	int index = YofBigScreen * m_bigScreenWidth + XofBigScreen;
	if(m_regions.size()<(index+1))
		return RectCoordinate();
	return m_regions[index].paintRegion;
}

const byte* BigScreenBackground::getDataOfBigscreenCell( int XOfBigScreen, int YOfBigScreen, int& bufLen )
{
	int index = YOfBigScreen * m_bigScreenWidth + XOfBigScreen;
	if(m_regions.size()<(index+1))
		return NULL;
	bufLen = m_regions[index].dataLength;
	return m_regions[index].pData;
}
	
IDisplayContentProvider* BigScreenBackground::getDataContentOfBigscreenCell(int XofBigScreen, int YofBigScreen) const
{
	int index = YofBigScreen * m_bigScreenWidth + XofBigScreen;
	if(m_regions.size()<(index+1))
		return NULL;
	return m_regions[index].m_dcproviders;
}

RECT_f BigScreenBackground::getDisplayRegOfBigscreenCell(int XofBigScreen, int YofBigScreen) const
{
	int index = YofBigScreen * m_bigScreenWidth + XofBigScreen;
	if(m_regions.size()<(index+1))
		return RECT_f();
	return m_regions[index].displayReg;
}

void BigScreenBackground::decodeBackgroundImageFile( const char* imageFileName, zRender::PIXFormat pixfomat )
{
	/*
	if(imageFileName==NULL)
		return;
	//读取文件
	std::locale::global(std::locale(""));
	std::ifstream imageFile(imageFileName, std::ios_base::in | std::ios_base::binary);
	std::locale::global(std::locale(""));
	if(imageFile.bad())
	{
		char msg[512] = {0};
		sprintf_s(msg, 512, "BigScreenBackground::decodeBackgroundImageFile Can not open Image file: %s", imageFileName);
		OutputDebugString(msg);
		return;
	}
	long begin = imageFile.tellg();
	imageFile.seekg(0, std::ios::end);
	long end = imageFile.tellg();
	long length = end - begin;
	imageFile.seekg(std::ios::beg);

	//加入图片最大大小的限制，100M
#define  SUPPORT_IMEGE_MAX_SIZE (100*1024*1024)
	if(length<=0 || SUPPORT_IMEGE_MAX_SIZE<length)
	{
		imageFile.close();
		char msg[128] = {0};
		sprintf_s(msg, 128, "BigScreenBackground::decodeBackgroundImageFile Image data block is %d , larger than 100MB or <=0", length);
		OutputDebugString(msg);
		return;
	}

	char* data = new char[length];
	if(data==NULL)
	{
		imageFile.close();
		return;
	}
	imageFile.read(data, length);
	imageFile.close();

	//解码，解码后数据可能很大如果图片分辨率很高的话
	SOA::Mirror::Decoder::IDecoder::decoderInit();
	SOA::Mirror::Decoder::IDecoder* decoder = SOA::Mirror::Decoder::IDecoder::createDecoder(CODEC_MJPEG, (SOA::Mirror::PIXFormat)pixfomat);
	if(decoder==NULL)
	{
		delete[] data;
		OutputDebugString("BigScreenBackground::decodeBackgroundImageFile 创建Decoder失败\n" );
		return;
	}
	if( decoder->decodeFrame((unsigned char*)data, length)<0 )
	{
		delete[] data;
		delete decoder;
		OutputDebugString("BigScreenBackground::decodeBackgroundImageFile failed to decompress image.\n");
		return;
	}
	delete[] data;
	SOA::Mirror::Decoder::MediaParam mp;
	decoder->getDecodedParam(mp);
	int datalength = mp.videoParam.width * mp.videoParam.height * SizeOfPixel[pixfomat];
	if(datalength==0)
	{
		delete decoder;
		OutputDebugString("BigScreenBackground::decodeBackgroundImageFile failed to get image size.\n");
		return;
	}

	m_decodedData = new byte[datalength];
	m_decodedDataLen = datalength;
	assert(m_decodedData);

	if( SOA::Mirror::Decoder::SUCCESS != decoder->getDecodedData(m_decodedData, m_decodedDataLen) )
	{
		delete decoder;
		delete[] m_decodedData;
		m_decodedDataLen = 0;
		OutputDebugString("BigScreenBackground::decodeBackgroundImageFile failed to get decoded data.\n");
		return;
	}

	m_imageWidth = mp.videoParam.width;
	m_imageHeight = mp.videoParam.height;
	delete decoder;
	*/
}

void BigScreenBackground::initBigScreenCellPaintRegion(const Size& cellRsolution)
{
	for(int i=0; i<m_bigScreenHeight; i++)
	{
		for(int j=0; j<m_bigScreenWidth; j++)
		{
			m_regions.push_back(BigScreenCellPaintRegion(j, i, cellRsolution.width, cellRsolution.height));
		}
	}
}

void BigScreenBackground::scaleTheImageFile( int dstImageWidth, int dstImageHeight )
{
	/*
	float rateX = (float)m_imageWidth / dstImageWidth;
	float rateY = (float)m_imageHeight / dstImageHeight;
	if(rateX>1 || rateY>1)
	{
		//保持纵横比的缩小
		float rate = rateX>rateY ? rateX : rateY;
		int scaledImageWidth = m_imageWidth / rate;
		int scaledImageHeight = m_imageHeight / rate;
		int disWidth = scaledImageWidth - dstImageWidth;
		int disHeight = scaledImageHeight - dstImageHeight;
		int dis = disWidth>disHeight ? disWidth : disHeight;
		dis = dis<0 ? 0 : dis;
		SOA::Mirror::Decoder::IVideoScaler* scaler = SOA::Mirror::Decoder::IVideoScaler::createVideoScaler(scaledImageWidth-dis, scaledImageHeight-dis, (SOA::Mirror::PIXFormat)m_pixformat);
		int scaledImageBufferLen = (scaledImageHeight-dis) * (scaledImageWidth-dis) * SizeOfPixel[m_pixformat];
		byte* scaledImageBuffer = new byte[scaledImageBufferLen];
		assert(scaledImageBuffer);
		SOA::Mirror::Decoder::DecodeResult result = scaler->scale(m_decodedData, m_imageWidth, m_imageHeight, (SOA::Mirror::PIXFormat)m_pixformat, scaledImageBuffer);
		if(result!=SOA::Mirror::Decoder::SUCCESS)
		{
			delete[] scaledImageBuffer;
		}
		else
		{
			delete[] m_decodedData;
			m_decodedData = scaledImageBuffer;
			m_decodedDataLen = scaledImageBufferLen;
			m_imageWidth = scaledImageWidth - dis;
			m_imageHeight = scaledImageHeight - dis;
		}
		delete scaler;
	}
	*/
}
