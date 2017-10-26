/*
 *	@file		MergedBigScreenBackground.h
 *	@data		2013-7-23 20:10
 *	@author		zhu qing quan
 *	@brief		合并模式的背景底图
 *  @Copyright	(C) 2013, by Ragile Corporation
 */

#pragma once
#ifndef _SOA_MIRROR_RENDER_MERGED_BIGSRCEEN_BACKGROUND_H_
#define _SOA_MIRROR_RENDER_MERGED_BIGSRCEEN_BACKGROUND_H_

#include "BigScreenBackground.h"

namespace SOA
{
namespace Mirror
{
namespace Render
{
	class MergedBigScreenBackground : public BigScreenBackground
	{
	public:
		MergedBigScreenBackground(const char* imageFileName,
			int  bigscreenWidth, int  bigscreenHeight,
			const Size& cellRsolution, zRender::PIXFormat pixfomat);

		virtual ~MergedBigScreenBackground();

		/**
		 * Method		freeBackgroundContent
		 * @brief		释放申请的内存，因为合并模式对图片进行了拆分，所以重新申请了内存空间
		 *				所以要重写该方法，释放内存
		 * @return		void
		 */
		void freeBackgroundContent();

		bool setBigscreenCellResolution(const Size& resolution);

	private:
		/**
		 * Method		splitBackgroundImageForBigscreenCell
		 * @brief		合并模式会对解码后的图片进行拆分
		 * @return		void
		 */
		void splitBackgroundImageForBigscreenCell();
	};
}
}
}

#endif // _SOA_MIRROR_RENDER_MERGED_BIGSRCEEN_BACKGROUND_H_