#pragma once
#include <vector>
#include "msgpack.hpp"
#include "MirrorTypes.h"
namespace SOA
{
	namespace Mirror
	{
		namespace RPC
		{
using namespace std;
enum BigScreenModeType
{
	BSMT_CONFIG=0,
	BSMT_SERVER=1
};

struct MirrorColor
{

	MirrorColor()
	{
		a = 0;
		r = 100;
		g = 255;
		b = 255;
	}

	MirrorColor(int i_a,int i_r,int i_g,int i_b)
	{
		a = (unsigned char)i_a;
		r = (unsigned char)i_r;
		g = (unsigned char)i_g;
		b = (unsigned char)i_b;
	}
	~MirrorColor()
	{

	}
	unsigned char a;
	unsigned char r;
	unsigned char g;
	unsigned char b;
	MSGPACK_DEFINE(a,r,g,b);
};

struct MaxDisplay
{
	MaxDisplay()
	{
		num = 1000;
		text = "连接路数超过服务器限制数。";
		style = "黑体";
		size = 24;
	}

	~MaxDisplay()
	{
		
	}
	int num;
	string text;
	string style;
	int size;
	MirrorColor color;
	MSGPACK_DEFINE(num,text,style,size,color);
};

struct BigScreenMonitor
{
	BigScreenMonitor()
	{

	}
	BigScreenMonitor(int monitorIndex,int x,int y,int w,int h)
		:index(monitorIndex)
		,BigScreenX(x)
		,BigScreenY(y)
		,width(w)
		,height(h)
		,id(-1)
	{

	}
	int index;
	int BigScreenX;
	int BigScreenY;
	int width;//WPF版本需要，其他不需要
	int height;//WPF版本需要，其他不需要
	LONGLONG id;
	MSGPACK_DEFINE(index,BigScreenX,BigScreenY);
};

struct BigScreenSetting
{
	BigScreenSetting()
	{

	}
	BigScreenSetting(long id,int mode,string ip,int port,int w,int h)
		:ID(id)
		,modeType(mode)
		,localIP(ip)
		,localPort(port)
		,width(w)
		,height(h)
	{

	}
	long ID;
	int modeType;//0：服务器版本，
	string localIP;
	int localPort;
	int width;
	int height;
	int monitorFrequency;
	vector<BigScreenMonitor> monitors;
	MaxDisplay maxDisplay;
	BigScreenConfig bigScreenConfig;
	BigScreenBackgroundMode bigscreenBackgroundMode;
	MSGPACK_DEFINE(ID,modeType,localIP,localPort,width,height,monitorFrequency,monitors,maxDisplay);
};
		}
	}
}