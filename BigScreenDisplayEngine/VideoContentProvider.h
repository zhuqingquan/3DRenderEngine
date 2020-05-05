/**
 *	@author		zhuqingquan
 *	@date		2015-1-10
 *	@name		VideoContentProvider.h
 *	@brief		显示内容的提供者的实现类之一，继承IDisplayContentProvider接口
 *				该类只是引用一帧图像数据的解码后像素数据，类中不拷贝原始数据。该类可以只是使用图像数据的一个区域用于显示，必须是矩形区域，不支持其他形状
 **/

#pragma once
#ifndef _SOA_MIRROR_RENDER_VIDEO_CP_H_
#define _SOA_MIRROR_RENDER_VIDEO_CP_H_

#include "IDisplayContentProvider.h"
#include <ConstDefine.h>

namespace zRender
{
	class IRawFrameTexture;
}

namespace SOA
{
namespace Mirror
{
namespace Render
{
	/**
	 *	@name		PictureTextureDataSource
	 *	@brief		图片内容的提供类的基类接口，各种图片数据可以通过继承该接口，使得图片可以在DxRender中显示
	 **/
	class PictureTextureDataSource : public zRender::TextureDataSource
	{
	public:
		/**
		 *	@name		PictureTextureDataSource
		 *	@brief		默认构造函数，所有数据成员都初始化成默认值，但继承类可以在构造之后修改数据成员的值
		 **/
		PictureTextureDataSource()
			: m_isUpdatedIdentify(0)
			, m_frameHeight(0), m_frameWidth(0), m_framePixFmt(zRender::PIXFMT_UNKNOW)
		{			
		}

		/**
		 *	@name		PictureTextureDataSource
		 *	@brief		构造函数，在构造时指定图片的分辨率
		 *	@param[in]	int width 图片宽
		 *	@param[in]	int height 图片高
		 **/
		PictureTextureDataSource(int width, int height)
			: m_isUpdatedIdentify(0)
			, m_frameHeight(height), m_frameWidth(width), m_framePixFmt(zRender::PIXFMT_UNKNOW)
			, m_Pitch(0)
		{
			
		}

		/**
		 *	@name		~PictureTextureDataSource
		 *	@brief		析构方法，所有成员变量设置成默认值
		 **/
		virtual ~PictureTextureDataSource() = 0
		{
			m_isUpdatedIdentify = 0;
			m_frameHeight = 0;
			m_frameWidth = 0;
			m_framePixFmt = zRender::PIXFMT_UNKNOW;
		}

		/**
		 *	@name		isUpdated
		 *	@brief		图片是否有更新，通过传入标识符来判断图片是否更新
		 *	@param[in]	int identify 图片是否有更新的标识符，如果类内部标识图片更新状态的整数比参数的值更大则认为图片已被更新
		 *				使用者可以在getData、copyDataToTexture中获取类内部标识图片更新状态的整数
		 *				然后用这个整数作为参数来判断上次调用了getData、copyDataToTexture之后图片有内容更新
		 *				使用者可以设置该参数为0，则该方法总是返回true
		 *	@return		true--图片内容有更新	false--图片内容没有更新
		 **/
		bool isUpdated(int identify) const {return m_isUpdatedIdentify>identify;}

		/**
		 *	@name		getTextureProfile
		 *	@brief		获取图片数据的相关信息，如数据的长度，数据的行宽，以及高
		 *				这里获取的是图片有效区域数据的信息，而不是整张图片的信息
		 *	@param[in]	const RECT_f& textureReg 需要获取的图片的区域，该参数表示用户想要获取图片部分区域的信息。
		 *				该参数为相对坐标，取值范围为[0,1]
		 *				整个图片作为一个整体，如果该参数为[0, 0, 0.5, 0.5]则获取的信息为图片左上角四分之一部分的宽高等。
		 *	@param[out]	int& dataLen 传出数据的长度
		 *	@param[out]	int& pitch 传出图片数据的行宽
		 *	@param[out]	int& width 传出图片的像素宽
		 *	@param[out]	int& height 传出图片的高
		 *	@param[out]	int& pixelFmt 传出图片的像素格式
		 *	@return		int 0--获取成功  <0--失败，可能是成员变量未初始化或者成员变量值不合法
		 **/
		int getTextureProfile(const zRender::RECT_f& textureReg, int& dataLen, int& yPitch, int& uPitch, int& vPitch, int& width, int& height, zRender::PIXFormat& pixelFmt)
		{
			if(textureReg.left<0 || textureReg.left>=1 || textureReg.top<0 || textureReg.top>=1
				|| textureReg.width() <= 0 || textureReg.height()<=0 || textureReg.right>1 ||textureReg.bottom>1)
			{
		#ifdef _DEBUG
				printf("Error in PictureTextureDataSource::getTextureProfile : param [textureReg] is invalid.(L=%f, R=%f, T=%f, B=%f)\n",
					textureReg.left, textureReg.right, textureReg.top, textureReg.bottom);
		#endif
				return -1;
			}
			if(m_framePixFmt==zRender::PIXFMT_UNKNOW || m_frameWidth==0 || m_frameHeight==0)
				return -2;
			float texReg_width = textureReg.width();
			float texReg_height = textureReg.height();
			float effReg_width = m_effectiveReg.width();
			float effReg_height = m_effectiveReg.height();
			float actual_width = texReg_width * effReg_width * m_frameWidth;
			float actual_height = texReg_height * effReg_height * m_frameHeight;
			width = actual_width;
			height = actual_height;
			yPitch = m_Pitch;//FRAMEPITCH(width, m_framePixFmt);
			dataLen = yPitch * height;
			pixelFmt = m_framePixFmt;
			return 0;
		}

		//virtual unsigned char* getData(int& dataLen, int& pitch, int& upitch, int& vpitch, int& width, int& height, zRender::PIXFormat& pixelFmt, RECT& effectReg, int& identify)
		virtual unsigned char* getData(int& dataLen, int& yPitch, int& uPitch, int& vPitch, int& width, int& height, zRender::PIXFormat& pixelFmt, RECT& effectReg, int& identify)
		{
			return NULL;
		}
		
		zRender::SharedTexture* getSharedTexture(RECT& effectReg, int& identify)
		{
			return NULL;
		}

		zRender::IRawFrameTexture* getTexture() {
			return NULL;
		}

		//virtual int copyDataToTexture(const zRender::RECT_f& textureReg, unsigned char* dstTextureData, int pitch, int height, int& identify) = 0;
		virtual int copyDataToTexture(const zRender::RECT_f& textureReg, unsigned char* dstTextureData, int pitch, int height, int& identify)
		{
			return 0;
		}

		/**
		 *	@name		setEffectiveReg
		 *	@brief		设置图片的有效区域，设置之后使用者只能获取到该有效区域部分的信息与数据
		 *	@param[in]	const zRender::RECT_f& textureEffectiveReg 图片有效区域
		 *				取值范围为[0,1]，为相对坐标
		 *	@return		0--成功  false--失败，参数不合法
		 **/
		int setEffectiveReg(const zRender::RECT_f& textureEffectiveReg)
		{
			if(textureEffectiveReg.left<0 || textureEffectiveReg.left>=1 || textureEffectiveReg.top<0 || textureEffectiveReg.top>=1
				|| textureEffectiveReg.width() <= 0 || textureEffectiveReg.height()<=0 || textureEffectiveReg.right>1 ||textureEffectiveReg.bottom>1)
			{
		#ifdef _DEBUG
				printf("Error in setEffectiveReg::setEffectiveReg : param [textureReg] is invalid.(L=%f, R=%f, T=%f, B=%f)\n",
					textureEffectiveReg.left, textureEffectiveReg.right, textureEffectiveReg.top, textureEffectiveReg.bottom);
		#endif
				return -1;
			}
			m_effectiveReg = textureEffectiveReg;
			m_isUpdatedIdentify++;
			return 0;
		}

		/**
		 *	@name		getEffectiveReg
		 *	@brief		获取图片的有效区域
		 *	@return		zRender::RECT_f 图片的有效区域，相对坐标，取值范围[0,1]
		 **/
		zRender::RECT_f getEffectiveReg() const
		{
			return m_effectiveReg;
		}

		/**
		 *	@name		getTextureCount
		 *	@brief		获取Element所需的TextureResource的个数
		 *	@return		zRender::VertexVector*
		 **/
		virtual int getTextureCount() const { return 1; }

		/**
		 *	@name		getTextureSourceDesc
		 *	@brief		获取下标为index的Texture对应的数据源的数据格式
		 *	@param[in]	int index TextureResource的下标
		 *	@param[out] TextureSourceDesc* srcDesc 保存数据源的数据格式
		 **/
		virtual int getTextureSourceDesc(int index, zRender::TextureSourceDesc* srcDesc) const
		{
			if (index > 0 || srcDesc == nullptr)
				return DXRENDER_RESULT_PARAM_INVALID;
			//fixme
			return DXRENDER_RESULT_OK;
		}

		/**
		 *	@name		updateTextures
		 *	@brief		将TextureDataSource中的数据更新到TextureResource中
		 *	@param[in]	TextureResource* textureArray TextureResource对象
		 *	@param[in]  int index 需要更新的Texture数据的下标Index
		 *	@param[out] unsigned int& newIdentify 成功更新后保存新的数据版本标识值，如果未更新则不更改
		 **/
		virtual int updateTextures(zRender::ITextureResource* textureArray, int index, unsigned int& newIdentify)
		{
			if (index < 0 || index>=getTextureCount() || textureArray == nullptr)
				return DXRENDER_RESULT_PARAM_INVALID;
			return DXRENDER_RESULT_OK;
		}

		/**
		 *	@name		isTextureUpdated
		 *	@brief		获取下标为index的Texture对应的数据源的数据是否发生了改变
		 *	@param[in]	int index TextureResource的下标
		 *	@param[in]  外部持有的最新更新的TextureResource的计数版本，如果Texture
		 *	@return		true--Texture数据源发生了更新 false--未更新
		 **/
		virtual bool isTextureUpdated(int index, unsigned int identify)
		{
			if (index < 0 || index>1)
				return false;
			return m_isUpdatedIdentify > identify;;

		}
	protected:
		int m_isUpdatedIdentify;
		int m_frameWidth;
		int m_frameHeight;
		int m_Pitch;
		zRender::PIXFormat m_framePixFmt;

	private:
		zRender::RECT_f m_effectiveReg;
	};
	
	/**
	 *	@name		VideoContentProvider
	 *	@brief		视频内容提供者的实现类，实现视频显示所需信息的提供
	 *				在该类中，提供的TextureDataSource对象可以是继承自PictureTextureDataSource接口的用于提供图片像素数据的类
	 *				提供的顶点信息都是顶点(0.0,0.0)与顶点(1.0, 1.0)所构成的矩形顶点信息
	 *				该实现类不提供Shader数据信息，使用默认的Shader文件进行显示
	 **/
	class VideoContentProvider : public zRender::IDisplayContentProvider
	{
	public:
		/**
		 *	@name		VideoContentProvider
		 *	@brief		构造函数，使用PictureTextureDataSource基类对象的指针作为参数
		 *	@param[in]	SOA::Mirror::Render::PictureTextureDataSource* PictureTextureDataSource基类对象的指针 
		 **/
		VideoContentProvider(SOA::Mirror::Render::PictureTextureDataSource* tds, const zRender::RECT_f& effctiveReg)
			: m_dataSrc(tds)
			, m_vv(new zRender::VertexVector(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST))
		{
			//默认的矩形的顶点
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
			m_vvCount = 1;

			if(m_dataSrc)
				m_dataSrc->setEffectiveReg(effctiveReg);
			m_vertexIdentify = 1;
		}

		/**
		 *	@name		~VideoContentProvider
		 *	@brief		释放相关资源
		 **/
		~VideoContentProvider()
		{
			delete m_vv;
			m_vv = NULL;
			m_vertexIdentify = 0;

			m_dataSrc = NULL;
		}

		/**
		 *	@name		isVertexUpdated
		 *	@brief		顶点信息是否已经更新
		 *	@param[in]	int identify 顶点信息是否有更新的标识符，如果类内部标识顶点信息更新状态的整数比参数的值更大则认为顶点信息已被更新
		 *				getVertexs可以获取到当前的顶点信息更新状态的标识符
		 *	@return		true--顶点信息已被更新  false--未更新
		 **/
		bool isVertexUpdated(int identify) const { return m_vertexIdentify>identify; }

		/**
		 *	@name		getVertexs
		 *	@brief		获取顶点信息
		 *	@param[out]	zRender::VertexVector** vv 传出顶点信息的数组，可能有多个数组
		 *	@param[out]	int& vvCount 顶点信息数组的个数
		 *	@param[in,out] int& identify 传入调用者当前所持有的顶点信息状态标识符，传出类内部的状态标识符
		 *	@return		int	0--成功  其他--失败
		 **/
		int getVertexs(zRender::VertexVector** vv, int& vvCount, int& identify)
		{
			if(vv==NULL || vvCount<1)
				return -1;
			if(m_vertexIdentify<identify)
				return 1;
			*vv = m_vv;
			vvCount = m_vvCount;
			identify = m_vertexIdentify;
			return 0;
		}

		/**
		 *	@name		getTextureDataSource
		 *	@brief		获取保存、提供视频数据的TextureDataSource对象指针
		 *	@return		zRender::TextureDataSource* 保存、提供视频数据的TextureDataSource对象指针
		 **/
		zRender::TextureDataSource* getTextureDataSource() { return m_dataSrc; }

		/**
		 *	@name		getShader
		 *	@brief		获取显示所需的Shader数据，在该类中直接返回NULL，使用默认的Shader
		 *	@return		返回NULL
		 **/
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
		SOA::Mirror::Render::PictureTextureDataSource* m_dataSrc;
		int m_vertexIdentify;
		zRender::VertexVector* m_vv;
		int m_vvCount;
	};

	class RawFrameRefData : public PictureTextureDataSource
	{
	public:
		RawFrameRefData(unsigned char* frameData, unsigned long dataLen,
						int width, int height, int pitch, zRender::PIXFormat pixfmt)
						: PictureTextureDataSource(width, height)
						, m_pFrameData(frameData), m_DataLen(dataLen)
		{
			m_framePixFmt = pixfmt;
			m_Pitch = pitch;
		}

		~RawFrameRefData() {}

		unsigned char* getData(int& dataLen, int& yPitch, int& uPitch, int& vPitch, int& width, int& height, zRender::PIXFormat& pixelFmt, RECT& effectReg, int& identify);
		//int getTextureProfile(const zRender::RECT_f& textureReg, int& dataLen, int& yPitch, int& uPitch, int& vPitch, int& width, int& height, zRender::PIXFormat& pixelFmt)
		//{
		//	return -1;
		//}

		int copyDataToTexture(const zRender::RECT_f& textureReg, unsigned char* dstTextureData, int pitch, int height, int& identify)
		{
			return -1;
		}

	private:
		unsigned char* m_pFrameData;
		unsigned long m_DataLen;

		bool isFrameParamValid() const
		{
			return m_pFrameData!=NULL && m_frameWidth>0 && m_frameHeight>0
				&& m_framePixFmt!=zRender::PIXFMT_UNKNOW 
				&& m_Pitch>=FRAMEPITCH(m_frameWidth, m_framePixFmt)
				&& m_DataLen >= m_Pitch*m_frameHeight;
		}
	};
}
}
}

#endif //_SOA_MIRROR_RENDER_FRAME_DATA_REF_CP_H_