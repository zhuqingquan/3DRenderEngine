#pragma once
#ifndef _SOA_MIRROR_RENDER_COMMAND_SHELL
#define _SOA_MIRROR_RENDER_COMMAND_SHELL

#include <iostream>
#include <set>

using namespace std;

struct Command
{
	std::string key;
	std::string detail;
	size_t linePos;

	Command(const std::string& k, const std::string& dtl, size_t lpos)
		: key(k)
		, detail(dtl)
		, linePos(lpos)
	{

	}
};

bool operator<(const Command& lObj, const Command& rObj)
{
	return lObj.linePos < rObj.linePos;
}

typedef std::set<Command> CommandMap;

void doCommand(const CommandMap& cmdMap);

void doCommand(std::istream& inputCommands)
{
	CommandMap preCmds;//从流中提取所有的命令语句，初步解析成特定的命令
	size_t lineCount = 0;
	while (inputCommands)
	{
		char line[1024] = { 0 };
		inputCommands.getline(line, sizeof(line));
		if (inputCommands)
			++lineCount;
		else
			break;
		if (line[0] == '/' && line[1] == '/')
			continue;
		std::string cmdTmp(line);
		std::string key;
		std::string::size_type beginPos = cmdTmp.find_first_not_of(' ');
		std::string::size_type spacePos;
		if (cmdTmp.npos != (spacePos = cmdTmp.find(' ', beginPos)))
		{
			key = cmdTmp.substr(beginPos, spacePos);
			preCmds.insert(Command(key, cmdTmp, lineCount));
		}
	}
	doCommand(preCmds);
}

struct ObjectInfo
{
	std::string typeName;
	std::string objName;
	void* Obj;
};

SOA::Mirror::Render::ScreenConfig sreencfg;

void doCommand(const CommandMap& cmdMap)
{
	CommandMap::const_iterator iter = cmdMap.begin();
	for (; iter != cmdMap.end(); iter++)
	{

	}
}

#endif//_SOA_MIRROR_RENDER_COMMAND_SHELL