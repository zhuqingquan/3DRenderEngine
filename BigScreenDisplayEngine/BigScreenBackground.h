/*
 *	@file		BigScreenBackground.h
 *	@data		2013-7-23 15:25
 *	@author		zhu qing quan
 *	@brief		BigScreen背景底图内容管理与提供的类
 *  @Copyright	(C) 2013, by Ragile Corporation
 */

#pragma once
#ifndef _SOA_MIRROR_RENDER_BIGSCREEN_BACKGROUND_H_
#define _SOA_MIRROR_RENDER_BIGSCREEN_BACKGROUND_H_

#include "BSFDataType.h"
#include "DxRenderCommon.h"
#include "IDisplayContentProvider.h"
#include <vector>

namespace SOA
{
namespace Mirror
{
namespace Render
{
	/**
	 * @name	BigScreenBackground
	 * @brief	电视墙背景底图内容管理和提供
	 */
	class BigScreenBackground
	{
	public:
		/**
		 * Method		BigScreenBackground
		 * @brief		构造函数
		 * @param[in]	const char * imageFileName 背景底图的文件，现在只支持JPEG图片
		 * @param[in]	int bigscreenWidth 电视墙的宽，如4*3
		 * @param[in]	int bigscreenHeight 电视墙的高，如4*3
		 * @param[in]	const Size & cellRsolution 电视墙中每个窗格的分辨率，现在认为每个窗格的分辨率都是一样的
		 *				如果存在不同分辨率的窗格，请使用独立模式的背景底图，因为合并模式可能看起来效果不好
		 * @param[in]	SOA::Mirror::PIXFormat pixfomat JPEG解码显示为底图后的像素格式
		 */
		BigScreenBackground(const char* imageFileName,
							int  bigscreenWidth, int  bigscreenHeight,
							const Size& cellRsolution, zRender::PIXFormat pixfomat);
		/**
		 * Method		~BigScreenBackground
		 * @brief		析构函数
		 */
		virtual ~BigScreenBackground();

		/**
		 * Method		setBigscreenCellResolution
		 * @brief		设置窗格的分辨率，未实现
		 *				到能够做到合并模式下窗格分辨率不同时也能有很好的显示效果则启用该方法
		 * @param[in]	const Size & resolution 窗格的分辨率
		 * @return		bool true--设置成功 false--设置失败
		 */
		virtual bool setBigscreenCellResolution(const Size& resolution) = 0;//设置单个窗格的分辨率

		/**
		 * Method		allocBackgroundContentForBigscreenCell
		 * @brief		解码图片，并将图片分割。为每个窗格分配显示内容的像素内存空间
		 * @return		void
		 */
		void allocBackgroundContentForBigscreenCell();

		/**
		 * Method		freeBackgroundContent
		 * @brief		将allocBackgroundContentForBigscreenCell方法中分配的显示内容所占用的内存空间释放
		 * @return		void
		 */
		virtual void freeBackgroundContent();

		/**
		 * Method		getBigscreenCellPaintRegion
		 * @brief		获取背景底图在单个窗格中的显示区域
		 * @param[in]	int XofBigScreen 窗格在电视墙中的X坐标
		 * @param[in]	int YofBigScreen 窗格在电视墙中的Y坐标
		 * @return		const SOA::Mirror::Render::RectCoordinate& 背景底图在窗格中的显示区域
		 */
		const SOA::Mirror::Render::RectCoordinate& getBigscreenCellPaintRegion(int XofBigScreen, int YofBigScreen);

		/**
		 * Method		getDataOfBigscreenCell
		 * @brief		获取单个窗格中显示的背景底图的内存块
		 * @param[in]	int XOfBigScreen 窗格在电视墙中的X坐标
		 * @param[in]	int YOfBigScreen 窗格在电视墙中的Y坐标
		 * @param[out]	int & bufLen 数据的长度（byte）
		 * @return		const byte* 指向背景底图的内存块的首地址
		 */
		const byte* getDataOfBigscreenCell(int XOfBigScreen, int YOfBigScreen, int& bufLen);

		/**
		 * Method		getDataContentOfBigscreenCell
		 * @brief		获取单个窗格中显示的背景底图的内容提供者，该内容提供者可用于在DxRender中显示
		 * @param[in]	int XOfBigScreen 窗格在电视墙中的X坐标
		 * @param[in]	int YOfBigScreen 窗格在电视墙中的Y坐标
		 * @return		zRender::IDisplayContentProvider* 背景底图的内容提供者 
		 */
		zRender::IDisplayContentProvider* getDataContentOfBigscreenCell(int XofBigscreen, int YofBigscreen) const; 

		/**
		 * Method		getDisplayRegOfBigscreenCell
		 * @brief		获取单个窗格中用于显示背景图片的区域在整个BigScreen中的坐标
		 * @param[in]	int XOfBigScreen 窗格在电视墙中的X坐标
		 * @param[in]	int YOfBigScreen 窗格在电视墙中的Y坐标
		 * @return		zRender::RECT_f 单个窗格中用于显示背景图片的区域在整个BigScreen中的坐标
		 */
		zRender::RECT_f getDisplayRegOfBigscreenCell(int XofBigscreen, int YofBigscreen) const;
	protected:

		struct BigScreenCellPaintRegion
		{
			int XOfBigScreen;
			int YOfBigScreen;
			int cellWidth;		//in pixel
			int cellHeight;		//in pixel
			SOA::Mirror::Render::RectCoordinate paintRegion;
			byte* pData;
			int dataLength;
			zRender::IDisplayContentProvider* m_dcproviders;
			zRender::RECT_f displayReg;

			BigScreenCellPaintRegion(int x, int y, int width, int height)
				: XOfBigScreen(x)
				, YOfBigScreen(y)
				, cellWidth(width)
				, cellHeight(height)
				, pData(NULL)
				, dataLength(0)
				, m_dcproviders(NULL)
			{
				paintRegion.left = paintRegion.right = paintRegion.bottom = paintRegion.top = -1;
			}
		};

		/**
		 * Method		splitBackgroundImageForBigscreenCell
		 * @brief		拆分背景底图，每个窗格独自保存自己的背景底图的内存块
		 * @return		void
		 */
		virtual void splitBackgroundImageForBigscreenCell() = 0;

		/**
		 * Method		scaleTheImageFile
		 * @brief		缩小图片，当图片分辨率大于窗格分辨率（独立模式）或者大于整个电视墙的分辨率（合并模式）时
		 *				缩小之后，图片的纵横比保持不变
		 * @param[in]	int dstImageWidth 缩小后图片的宽
		 * @param[in]	int dstImageHeight 缩小后图片的高
		 * @return		void
		 */
		void scaleTheImageFile(int dstImageWidth, int dstImageHeight);
	private:

		/**
		 * Method		decodeBackgroundImageFile
		 * @brief		读取文件内容并解码
		 * @param[in]	const char * imageFileName 背景底图文件名
		 * @param[in]	SOA::Mirror::PIXFormat pixfomat 解码后像素的格式
		 * @return		void
		 */
		void decodeBackgroundImageFile(const char* imageFileName, zRender::PIXFormat pixfomat);

		/**
		 * Method		initBigScreenCellPaintRegion
		 * @brief		初始化每个窗格绘制区域的信息
		 *				只是初始化，要等splitBackgroundImageForBigscreenCell方法之后这些区域信息才有效
		 * @param[in]	const Size & cellRsolution
		 * @return		void
		 */
		void initBigScreenCellPaintRegion(const Size& cellRsolution);

	protected:
		byte* m_decodedData;								//解码后数据首地址
		int m_decodedDataLen;								//解码后数据长度
		int m_bigScreenWidth;								//电视墙的宽，如4*3
		int m_bigScreenHeight;								//电视墙的高，如4*3
		int m_imageWidth;									//背景底图的宽
		int m_imageHeight;									//背景底图的高
		bool m_isSplited;									//标识是否已拆分图片到每个窗格
		zRender::PIXFormat m_pixformat;					//图片解码后的像素格式
		const Size m_bigscreenCellResolution;				//每个窗格的分辨率	
		std::vector<BigScreenCellPaintRegion> m_regions;	//所有窗格的底图显示区域的信息

	};

}
}
}

#endif // _SOA_MIRROR_RENDER_BIGSCREEN_BACKGROUND_H_