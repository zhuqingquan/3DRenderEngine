#pragma once

#ifndef _SOA_MIRROR_RPC_OSD_TEXT_H_
#define _SOA_MIRROR_RPC_OSD_TEXT_H_

#include "msgpack.hpp"
#include "OSDTextType.h"

namespace SOA
{
namespace Mirror
{
namespace RPC
{
	struct OSDText
	{
		std::string text;
		int type;

		OSDText(const std::string& textCt, OSDTextType textType)
			: text(textCt)
			, type(textType)
		{

		}
		OSDText()
			: type(OSDTextType::Text)
		{

		}
		OSDText(const OSDText& obj)
			: text(obj.text)
			, type(obj.type)
		{

		}

		MSGPACK_DEFINE(text, (int)type);
	};


	//OSDText类的中间传递类
	//因为托管的std::string传递到非托管DLL中会损失内容，所以添加此中间传递类
	struct OSDTextMedium
	{
		char* text;
		OSDTextType type;
	};

	/**
	 * @name	Shadow
	 * @brief	文字OSD阴影的参数
	 */
	struct Shadow
	{
		unsigned int		color;				//背景颜色
		unsigned int		skewingPixels;		//背景偏移的像素个数
		unsigned float		angle;				//背景偏移的角度

		MSGPACK_DEFINE(color, skewingPixels, angle);
	};
}		
}
}
#endif 