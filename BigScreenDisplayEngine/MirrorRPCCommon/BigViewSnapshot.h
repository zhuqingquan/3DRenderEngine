/*
 *	@file		BigViewSnapshot.h
 *	@data		2013-4-2 11:24
 *	@author		zhu qing quan
 *	@brief		BigView抓图生成的图片数据
 *  @Copyright	(C) 2012, by Ragile Corporation
 */

#pragma once
#ifndef _SOA_MIRROR_RPC_BIGVIEW_SNAPSHOT_H_
#define _SOA_MIRROR_RPC_BIGVIEW_SNAPSHOT_H_

#include "msgpack.hpp"

namespace SOA
{
namespace Mirror
{
namespace RPC
{
	/**
	 *	BigView抓图的文件类型，现在只支持BMP、JPEG两种
	 *  为了避免在枚举之间的转换，这个枚举的值与SOA::Mirror::Decoder::ImageFormat枚举的值保持一致
	 */
	typedef enum BigViewSnaphotFormat
	{
		BVSNPFMT_UNKNOW = 0,		//未知
		BVSNPFMT_BMP = 1,			//BMP
		BVSNPFMT_JPEG = 3,			//JPEG
	};

	/**
	 * @name	BigViewSnapshot
	 * @brief	bigview截图图片数据
	 */
	struct BigViewSnapshot
	{
		msgpack::type::raw_ref data;		///< 数据
		unsigned long length;				///< 长度
		int format;							///< 类型 BigViewSnaphotFormat

		BigViewSnapshot()
			: length(0)
			, format(BVSNPFMT_UNKNOW)
			, DataFreeFunc(NULL)
		{

		}

		~BigViewSnapshot()
		{
			if(DataFreeFunc!=NULL)
				DataFreeFunc((char*)data.ptr);
		}

		void (*DataFreeFunc)(void* pData);		///< 释放数据的函数指针

		MSGPACK_DEFINE(format, length, data);
	};
}
}
}

#endif // _SOA_MIRROR_RPC_BIGVIEW_SNAPSHOT_H_