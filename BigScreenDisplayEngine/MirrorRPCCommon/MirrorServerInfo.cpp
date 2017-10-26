#include "MirrorServerInfo.h"

using namespace SOA::Mirror::RPC;

std::vector<std::string> SOA::Mirror::RPC::MirrorServerInfo_map::getLocalIPs() const
{
	std::vector<std::string> strIPsVec;
	MirrorServerInfo_map::PropertyCollection::const_iterator ipIter = serverPropertys.find("IPs");
	if(ipIter!=serverPropertys.end())
	{
		strIPsVec = splitString(ipIter->second, ';');
	}
	return strIPsVec;
}

int SOA::Mirror::RPC::MirrorServerInfo_map::getPort() const
{
	int port = -1;
	MirrorServerInfo_map::PropertyCollection::const_iterator portIter = serverPropertys.find("port");
	if(portIter!=serverPropertys.end())
	{
		std::string strPort = portIter->second;
		port = atoi(strPort.c_str());
	}
	return port;
}

std::string SOA::Mirror::RPC::MirrorServerInfo_map::getName() const
{
	std::string name;
	MirrorServerInfo_map::PropertyCollection::const_iterator nameIter = serverPropertys.find("name");
	if(nameIter!=serverPropertys.end())
	{
		name = nameIter->second;
	}
	return name;
}

std::string SOA::Mirror::RPC::MirrorServerInfo_map::getGUID() const
{
	std::string strGUID;
	MirrorServerInfo_map::PropertyCollection::const_iterator guidIter = serverPropertys.find("guid");
	if(guidIter!=serverPropertys.end())
	{
		strGUID = guidIter->second;
	}
	return strGUID;
}

std::vector<std::string> SOA::Mirror::RPC::MirrorServerInfo_map::getMACs() const
{
	std::vector<std::string> strMACsVec;
	MirrorServerInfo_map::PropertyCollection::const_iterator macIter = serverPropertys.find("mac");
	if(macIter!=serverPropertys.end())
	{
		std::string strMacList = macIter->second;
		strMACsVec = splitString(strMacList, ';');
	}
	return strMACsVec;
}

void SOA::Mirror::RPC::MirrorServerInfo_map::setIPs( const std::vector<std::string>& strIPsVec )
{
	std::string strIPs;
	for(int i=0; i<strIPsVec.size(); i++)
	{
		strIPs = strIPs + strIPsVec[i];
		if(i!=strIPsVec.size()-1)
			strIPs = strIPs + ";";
	}
	serverPropertys["IPs"] = strIPs;
}

void SOA::Mirror::RPC::MirrorServerInfo_map::setPort( int port )
{
	char strPort[32] = {0};
	sprintf_s(strPort, 32, "%d", port);
	if(strlen(strPort)>0)
	{
		serverPropertys["port"] = strPort;
	}
}

void SOA::Mirror::RPC::MirrorServerInfo_map::setName( const char* name )
{
	if(name!=NULL || strlen(name)>0)
		serverPropertys["name"] = name;
}

void SOA::Mirror::RPC::MirrorServerInfo_map::setGUID( const char* strGuid )
{
	if(strGuid!=NULL || strlen(strGuid)>0)
		serverPropertys["guid"] = strGuid;
}

void SOA::Mirror::RPC::MirrorServerInfo_map::setMACs( const std::vector<std::string>& strMACsVec )
{
	std::string strIPs;
	for(int i=0; i<strMACsVec.size(); i++)
	{
		strIPs = strIPs + strMACsVec[i];
		if(i!=strMACsVec.size()-1)
			strIPs = strIPs + ";";
	}
	serverPropertys["mac"] = strIPs;
}

void SOA::Mirror::RPC::MirrorServerInfo_map::setProperty( const std::string& propertyName, const std::string& propertyValue )
{
	serverPropertys[propertyName] = propertyValue;
}

std::vector<std::string> SOA::Mirror::RPC::splitString( const std::string& src, char seq )
{
	std::vector<std::string> resultVec;
	if(src.empty() || seq<=0)
		return resultVec;
	const char* pstrSrc = src.c_str();
	int totalLenght = src.length();
	int lastPos = 0;
	int curPos = 0;
	while(curPos<totalLenght)
	{
		if(*(pstrSrc+curPos)==seq)
		{
			std::string oneSubstr = src.substr(lastPos, curPos-lastPos);
			if(!oneSubstr.empty())
				resultVec.push_back(oneSubstr);
			lastPos = curPos + 1;
		}
		++curPos;
	}
	if(lastPos<totalLenght)
	{
		std::string oneSubstr = src.substr(lastPos, totalLenght-lastPos);
		if(!oneSubstr.empty())
			resultVec.push_back(oneSubstr);
	}
	return resultVec;
}
