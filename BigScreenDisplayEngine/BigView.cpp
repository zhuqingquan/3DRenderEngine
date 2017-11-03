#include "BigView.h"
#include "BigViewportPartition.h"
#include <fstream>
#include <assert.h>
#include "YUVTexture_Packed.h"
#include "VideoTextureDataSource.h"

using namespace SOA::Mirror::Render;
using namespace zRender;

class VideoContentProvider : public IDisplayContentProvider
{
public:
	VideoContentProvider(const RECT_f& effctiveReg)
		: m_srcUpdater(NULL)
		, m_dataSrc(NULL)
	{
		m_vv = new VertexVector(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		zRender::Vertex ver[4];
		ver[0].Pos = XMFLOAT3(0.0, 0.0, 0.0);
		ver[0].Tex = XMFLOAT2(0.0, 1.0);
		ver[1].Pos = XMFLOAT3(1.0, 0.0, 0.0);
		ver[1].Tex = XMFLOAT2(1.0, 1.0);
		ver[2].Pos = XMFLOAT3(0.0, 1.0, 0.0);
		ver[2].Tex = XMFLOAT2(0.0, 0.0);
		ver[3].Pos = XMFLOAT3(1.0, 1.0, 0.0);
		ver[3].Tex = XMFLOAT2(1.0, 0.0);
		UINT index[6] = {0, 2, 3, 0, 3, 1};
		m_vv->addVertexs(ver, 4, index, 6);

		//PictureTextureDataSource* ptds = new PictureTextureDataSource("D:\\´úÂëºÚ¶´\\datasource\\Frame-720X576.yuy2", 720, 576, PIXFMT_YUY2);
		//PictureTextureDataSource* ptds = new PictureTextureDataSource("D:\\´úÂëºÚ¶´\\datasource\\new\\1920X1080.rgb", 1920, 1080, PIXFMT_A8R8G8B8);
		//PictureTextureDataSource* ptds = new PictureTextureDataSource("D:\\´úÂëºÚ¶´\\datasource\\frame-i420.i420", 720, 576, PIXFMT_YUV420P);
		//PictureTextureDataSource* ptds = new PictureTextureDataSource("D:\\´úÂëºÚ¶´\\datasource\\frame-nv12.nv12", 720, 576, PIXFMT_NV12);
		//ptds->setEffectiveReg(effctiveReg);
		//m_dataSrc  = ptds;

		//VideoTextureDataSource* vtds = new VideoTextureDataSource("d:\\InsideMoveVtc.yuv", zRender::PIXFMT_YUY2, 1920, 1080, 1920*2);
		//vtds->setEffectiveReg(effctiveReg);
		//m_dataSrc  = vtds;
		//m_srcUpdater = new VideoTextureSourceUpdater(vtds, 25);
		//m_srcUpdater->start();

		m_vertexIdentify = 1;
	}

	~VideoContentProvider()
	{
		delete m_vv;
		m_vv = NULL;
		m_vertexIdentify = 0;

		if(m_srcUpdater)
		{
			m_srcUpdater->stop();
			delete m_srcUpdater;
			m_srcUpdater = NULL;
		}

		//delete m_dataSrc;
		m_dataSrc = NULL;
	}

	class PictureTextureDataSource : public TextureDataSource
	{
	public:
		PictureTextureDataSource(const char* filePathName, int width, int height, PIXFormat pixfmt)
			: m_isUpdatedIdentify(0)
			, m_frameData(NULL), m_frameHeight(0), m_frameWidth(0), m_framePixFmt(PIXFMT_UNKNOW)
		{
			std::ifstream yuy2FileStream( filePathName, std::ios::in | std::ios::binary);
			if(!yuy2FileStream)
				return;
			int yuy2FrameDataLen = FRAMESIZE(width, height, pixfmt);
			if(yuy2FrameDataLen<=0)
			{
				yuy2FileStream.close();
				return;
			}
			m_frameWidth = width;//720;
			m_frameHeight = height;//576;
			m_framePixFmt = pixfmt;
			m_frameData = (unsigned char*)malloc(yuy2FrameDataLen);
			assert(m_frameData);
			yuy2FileStream.read((char*)m_frameData, yuy2FrameDataLen);
			yuy2FileStream.close();
			m_isUpdatedIdentify++;
		}

		~PictureTextureDataSource()
		{
			free(m_frameData);
			m_frameData = NULL;
		}

		bool isUpdated(int identify) const {return m_isUpdatedIdentify>identify;}

		int getTextureProfile(const zRender::RECT_f& textureReg, int& dataLen, int& yPitch, int& uPitch, int& vPitch, int& width, int& height, zRender::PIXFormat& pixelFmt)
		{
			if(textureReg.left<0 || textureReg.left>=1 || textureReg.top<0 || textureReg.top>=1
				|| textureReg.width() <= 0 || textureReg.height()<=0 || textureReg.right>1 ||textureReg.bottom>1)
			{
		#ifdef _DEBUG
				printf("Error in DisplayElement::setTextureDataSource : param [textureReg] is invalid.(L=%f, R=%f, T=%f, B=%f)\n",
					textureReg.left, textureReg.right, textureReg.top, textureReg.bottom);
		#endif
				return -1;
			}
			if(m_frameData==NULL || m_frameWidth==0 || m_frameHeight==0)
				return -2;
			float texReg_width = textureReg.width();
			float texReg_height = textureReg.height();
			float effReg_width = m_effectiveReg.width();
			float effReg_height = m_effectiveReg.height();
			float actual_width = texReg_width * effReg_width * m_frameWidth;
			float actual_height = texReg_height * effReg_height * m_frameHeight;
			width = actual_width;
			height = actual_height;
			yPitch = width * 2;
			dataLen = yPitch * height;
			pixelFmt = m_framePixFmt;
			return 0;
		}

		//unsigned char* getData(int& dataLen, int& pitch, int& width, int& height, int& pixelFmt)
		unsigned char* getData(int& dataLen, int& yPitch, int& uPitch, int& vPitch, int& width, int& height, zRender::PIXFormat& pixelFmt, RECT& effectReg, int& identify)
		{
			if(m_frameData==NULL || m_frameWidth==0 || m_frameHeight==0)
				return NULL;
			if(m_isUpdatedIdentify<=identify)
				return NULL;
			dataLen = FRAMESIZE(m_frameWidth, m_frameHeight, m_framePixFmt);
			if(dataLen<=0)
				return NULL;
			yPitch = FRAMEPITCH(m_frameWidth, m_framePixFmt);
			width = m_frameWidth;
			height = m_frameHeight;
			pixelFmt = m_framePixFmt;
			
			effectReg.left = m_frameWidth * m_effectiveReg.left + 0.5;
			effectReg.right = m_frameWidth * m_effectiveReg.right + 0.5;
			effectReg.top = m_frameHeight * m_effectiveReg.top + 0.5;
			effectReg.bottom = m_frameHeight * m_effectiveReg.bottom + 0.5;
			identify = m_isUpdatedIdentify;
			return m_frameData;
		}
		
		int copyDataToTexture(const RECT_f& textureReg, unsigned char* dstTextureData, int pitch, int height, int& identify)
		{
			if(m_frameData==NULL || m_frameWidth==0 || m_frameHeight==0)
				return -1;

			if(m_isUpdatedIdentify<=identify)
				return 1;

			float texReg_width = textureReg.width();
			float texReg_height = textureReg.height();
			float effReg_width = m_effectiveReg.width();
			float effReg_height = m_effectiveReg.height();
			float actual_width = texReg_width * effReg_width * m_frameWidth;
			float actual_height = texReg_height * effReg_height * m_frameHeight;
			if(pitch<FRAMEPITCH(actual_width, m_framePixFmt) || height<actual_height || NULL==dstTextureData)
			{
#ifdef _DEBUG
				printf("Error copyDataToTexture : param invalid.(pData=%d, pitch=%d, H=%d)\n",
					(int)dstTextureData, pitch, height);
#endif
				return -2;
			}
			int dataPitch = FRAMEPITCH(m_frameWidth, m_framePixFmt);
			//float fStartPosHrz = m_frameWidth * m_effectiveReg.left + actual_width * textureReg.left + 0.5;
			//float fStartPosVtc = m_frameHeight * m_effectiveReg.top + actual_height * textureReg.top + 0.5;
			float fStartPosHrz = m_frameWidth * (m_effectiveReg.left + m_effectiveReg.width()*textureReg.left) + 0.5;
			float fStartPosVtc = m_frameHeight * (m_effectiveReg.top + m_effectiveReg.height() * textureReg.top) + 0.5;
			int iStartPosHrz = fStartPosHrz;
			int iStartPosVtc = fStartPosVtc;
			int iEndPosVtc = iStartPosVtc+actual_height;
			int dataLenCopyed = FRAMEPITCH(actual_width, m_framePixFmt);//actual_width*2;
			for(int iVtc=iStartPosVtc; iVtc<iEndPosVtc; iVtc++)
			{				
				int dataPos = iVtc * dataPitch + FRAMEPITCH(iStartPosHrz, m_framePixFmt);
				unsigned char* pVtcData = m_frameData + dataPos;
				memcpy(dstTextureData, pVtcData, dataLenCopyed);
				dstTextureData += pitch;
			}
			return 0;
		}

		int setEffectiveReg(const RECT_f& textureEffectiveReg)
		{
			if(textureEffectiveReg.left<0 || textureEffectiveReg.left>=1 || textureEffectiveReg.top<0 || textureEffectiveReg.top>=1
				|| textureEffectiveReg.width() <= 0 || textureEffectiveReg.height()<=0 || textureEffectiveReg.right>1 ||textureEffectiveReg.bottom>1)
			{
		#ifdef _DEBUG
				printf("Error in DisplayElement::setTextureDataSource : param [textureReg] is invalid.(L=%f, R=%f, T=%f, B=%f)\n",
					textureEffectiveReg.left, textureEffectiveReg.right, textureEffectiveReg.top, textureEffectiveReg.bottom);
		#endif
				return -1;
			}
			m_effectiveReg = textureEffectiveReg;
			m_isUpdatedIdentify++;
			return 0;
		}
	private:
		int m_isUpdatedIdentify;
		RECT_f m_effectiveReg;
		unsigned char* m_frameData;
		int m_frameWidth;
		int m_frameHeight;
		PIXFormat m_framePixFmt;
	};

	bool isVertexUpdated(int identify) const { return m_vertexIdentify>identify; }

	int getVertexs(VertexVector** vv, int& vvCount, int& identify)
	{
		if(vv==NULL || vvCount<1)
			return -1;
		if(m_vertexIdentify<identify)
			return 1;
		*vv = m_vv;
		vvCount = 1;
		identify = m_vertexIdentify;
		return 0;
	}

	bool setTextureDataSource(zRender::TextureDataSource* textureDataSrc) 
	{
		m_dataSrc = textureDataSrc;
		return true;
	}

	TextureDataSource* getTextureDataSource() { return m_dataSrc; }
	void* getShader() { return NULL; }

	void increaseAuthorization()
	{
		if(m_dataSrc!=NULL)
			m_dataSrc->increaseAuthorization();
	}
	void decreaseAuthorization()
	{
		if(m_dataSrc!=NULL)
			m_dataSrc->decreaseAuthorization();
	}
private:
	TextureDataSource* m_dataSrc;
	int m_vertexIdentify;
	VertexVector* m_vv;
	int m_vvCount;
	VideoTextureSourceUpdater* m_srcUpdater;
};

BigView::BigView(const zRender::RECT_f& effectiveReg)
	: m_effectiveReg(effectiveReg)
	, m_contentProvider(NULL)
{
}

BigView::~BigView()
{
	releaseContentProvider();
}

zRender::IDisplayContentProvider* BigView::applyAuthorization(BigViewportPartition* bvpp)
{
	if(m_effectiveReg.width()<=0 || m_effectiveReg.height()<=0
		|| m_effectiveReg.left <0 || m_effectiveReg.top<0)
	{
#ifdef _DEBUG
		printf("Error in BigView::applyAuthorization : effective region of this BigView is invalid.(L=%f, T=%f, R=%f, B=%f)\n",
			m_effectiveReg.left, m_effectiveReg.top, m_effectiveReg.right, m_effectiveReg.bottom);
#endif
		return NULL;
	}
	if(m_contentProvider==NULL)
		if(0!=createContentProvider())
			return NULL;
	m_authorizatedViewportPartion.push_back(bvpp);
	m_contentProvider->increaseAuthorization();
	return m_contentProvider;
}

int BigView::releaseAutorization(BigViewportPartition* bvpp)
{
	bool isFinded = false;
	std::list<BigViewportPartition*>::const_iterator iter = m_authorizatedViewportPartion.begin();
	for (; iter!=m_authorizatedViewportPartion.end(); iter++)
	{
		if(*iter==bvpp)
		{
			isFinded = true;
			m_authorizatedViewportPartion.erase(iter);
			m_contentProvider->decreaseAuthorization();
			break;
		}
	}
	return isFinded ? 0 : -1;
}

int BigView::setEffectiveReg(const zRender::RECT_f& effectiveReg)
{
	m_effectiveReg = effectiveReg;
	return 0;
}

bool BigView::isNeedShow() const
{
	bool isNeedShow = false;
	std::list<BigViewportPartition*>::const_iterator iter = m_authorizatedViewportPartion.begin();
	for (; iter!=m_authorizatedViewportPartion.end(); iter++)
	{
		BigViewportPartition* bvvp = *iter;
		if(bvvp->isValid())
		{
			isNeedShow = true;
			break;
		}
	}
	return isNeedShow;
}

bool SOA::Mirror::Render::BigView::attachTextureSource(zRender::TextureDataSource * textureDataSrc)
{
	if (m_contentProvider == NULL)
		if (0 != createContentProvider())
			return false;
	if (m_contentProvider)
	{
		VideoContentProvider* vcp = dynamic_cast<VideoContentProvider*>(m_contentProvider);
		vcp->setTextureDataSource(textureDataSrc);
	}
	return true;
}

int BigView::createContentProvider()
{
	if(m_contentProvider)
		return 0;
	m_contentProvider = new VideoContentProvider(m_effectiveReg);
	return m_contentProvider ? 0 : -1;
}

void BigView::releaseContentProvider()
{
	if(m_contentProvider)
	{
		delete m_contentProvider;
		m_contentProvider = NULL;
	}
}