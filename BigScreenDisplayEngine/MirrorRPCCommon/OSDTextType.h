#pragma once

#ifndef _OSD_TEXT_TYPE_H_
#define _OSD_TEXT_TYPE_H_

namespace SOA
{
namespace Mirror
{
namespace RPC
{
	enum OSDTextType
	{
		//OSD 为静态文本
		Text=0,
		//OSD 为时间，MirrorServer获取机器系统时间显示
		DateTime=1
	};
}
}
}
#endif 