#include "BigScreenInfo.h"

using namespace SOA::Mirror::RPC;


SOA::Mirror::RPC::BigScreenInfo::BigScreenInfo()
{
	windowInfos = new std::vector<WindowInfo>();
}

SOA::Mirror::RPC::BigScreenInfo::~BigScreenInfo()
{
	if(windowInfos!=NULL)
		delete windowInfos;
}

const BigScreenInfo& BigScreenInfo::operator=(const BigScreenInfo& info)
{
	this->width = info.width;
	this->height = info.height;
	this->windowInfos = new std::vector<WindowInfo>();
	*this->windowInfos = *info.windowInfos;
	return *this;
}

void SOA::Mirror::RPC::BigScreenInfo::addWindowInfo( WindowInfo& info )
{
	windowInfos->push_back(info);
}

SOA::Mirror::RPC::WindowInfo::WindowInfo()
{

}

SOA::Mirror::RPC::WindowInfo::~WindowInfo()
{

}
