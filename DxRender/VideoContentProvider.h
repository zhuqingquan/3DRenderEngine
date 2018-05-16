/**
 *	@author		zhuqingquan
 *	@date		2015-1-10
 *	@name		VideoContentProvider.h
 *	@brief		显示内容的提供者的实现类之一，继承IDisplayContentProvider接口
 *				该类只是引用一帧图像数据的解码后像素数据，类中不拷贝原始数据。该类可以只是使用图像数据的一个区域用于显示，必须是矩形区域，不支持其他形状
 **/

#pragma once
#ifndef _ZRENDER_VIDEO_CP_H_
#define _ZRENDER_VIDEO_CP_H_

#include "IDisplayContentProvider.h"

namespace zRender
{
	class DX_ZRENDER_EXPORT_IMPORT VideoContentProvider : public IDisplayContentProvider
	{
	public:
		VideoContentProvider(TextureDataSource* dataSrc);

		~VideoContentProvider();

		bool isVertexUpdated(int identify) const { return m_vertexIdentify>identify; }

		int getVertexs(VertexVector** vv, int& vvCount, int& identify);

		virtual int setRotation(int rotate);
		virtual int setRotation_x(int rotate);
		virtual int setRotation_y(int rotate);

		TextureDataSource* getTextureDataSource() { return m_dataSrc; }
		void* getShader();

		void increaseAuthorization();

		void decreaseAuthorization();
	private:
		TextureDataSource* m_dataSrc;
		int m_vertexIdentify;
		VertexVector* m_vv;
		int m_vvCount;

		int m_rotate;
		int m_rotateX;
		int m_rotateY;
	};

}

#endif //_ZRENDER_FRAME_DATA_REF_CP_H_