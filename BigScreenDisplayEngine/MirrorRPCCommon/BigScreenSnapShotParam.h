#include "msgpack.hpp"
#include "MirrorTypes.h"
struct BigScreenPaneInfo
{
	BigScreenPaneInfo()
	{
		x = -1;
		y = -1;
		pixFormat = SOA::Mirror::PIXFormat::PIXFMT_UNKOWN;
		width = 0;
		height = 0;	
	}
	~BigScreenPaneInfo()
	{
		x = -2;
		y = -2;
		pixFormat = SOA::Mirror::PIXFormat::PIXFMT_UNKOWN;
		width = 0;
		height = 0;
	}
	int x;	//窗格在Screen中的位置。
	int y;	//窗格在Screen中的位置。
	int pixFormat;	//快照的数据格式。
	int width;	//窗格的分辨率
	int height;	//窗格的分辨率
	MSGPACK_DEFINE(x,y,pixFormat,width,height);
};

struct BigScreenSnapShotParam
{
	BigScreenSnapShotParam()
	{
		snapTime=0;
	}

	bool isValid()
	{
		return snapTime>0;
	}

	BigScreenPaneInfo panelInfo;
	long snapTime;
	MSGPACK_DEFINE(panelInfo,snapTime);
};