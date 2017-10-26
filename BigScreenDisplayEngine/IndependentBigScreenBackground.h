/*
 *	@file		IndependentBigScreenBackground.h
 *	@data		2013-7-23 19:00
 *	@author		zhu qing quan
 *	@brief		独立模式的背景底图
 *  @Copyright	(C) 2013, by Ragile Corporation
 */

#pragma once
#ifndef _SOA_MIRROR_RENDER_INDEPENDENT_BIGSCREEN_BACKGROUND_H_
#define _SOA_MIRROR_RENDER_INDEPENDENT_BIGSCREEN_BACKGROUND_H_

#include "BigScreenBackground.h"

namespace SOA
{
namespace Mirror
{
namespace Render
{
	/**
	 * @name	IndependentBigScreenBackground
	 * @brief	独立模式的背景底图
	 */
	class IndependentBigScreenBackground : public BigScreenBackground
	{
	public:
		IndependentBigScreenBackground(const char* imageFileName,
										int  bigscreenWidth, int  bigscreenHeight,
										const Size& cellRsolution, zRender::PIXFormat pixfomat);

		virtual ~IndependentBigScreenBackground();

		bool setBigscreenCellResolution(const Size& resolution);

	private:
		/**
		 * Method		splitBackgroundImageForBigscreenCell
		 * @brief		在独立模式中只做图片的缩小操作，不进行拆分
		 * @return		void
		 */
		void splitBackgroundImageForBigscreenCell();
	};
}
}
}

#endif // _SOA_MIRROR_RENDER_INDEPENDENT_BIGSCREEN_BACKGROUND_H_