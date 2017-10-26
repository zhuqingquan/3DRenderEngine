
#pragma once
#ifndef _SOA_MIRROR_RPC_BIGFONT_H_
#define _SOA_MIRROR_RPC_BIGFONT_H_

#include "msgpack.hpp"

namespace SOA
{
	namespace Mirror
	{
		namespace RPC
		{
enum FontWeightType
{
	FWT_DONTCARE		 = 0,
	FWT_THIN			 = 100,
	FWT_EXTRALIGHT       = 200,
	FWT_LIGHT            = 300,
	FWT_NORMAL           = 400,
	FWT_MEDIUM           = 500,
	FWT_SEMIBOLD         = 600,
	FWT_BOLD             = 700,
	FWT_EXTRABOLD        = 800,
	FWT_HEAVY            = 900,
};
struct MirrorFont
{
public:
	int width;//字符的宽度，像素
	int height;//字符的高度，像素。
	bool italic;//是否斜体
	//WeightType weightType;//正常，粗体等。
	//FontType fontType;//字体
	int fontWeightType;//正常，粗体等。
	//FontType fontType;//字体
	string pFacename;
	MSGPACK_DEFINE(width,height,italic,fontWeightType,pFacename);
};
		}
	}
}
#endif