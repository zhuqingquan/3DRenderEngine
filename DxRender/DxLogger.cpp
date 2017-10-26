#include "DXLogger.h"

using namespace zRender;

static DxLogFunc g_LogFunc = NULL;

DxLogFunc zRender::setDxLogFunc(DxLogFunc logFunc)
{
	DxLogFunc old = g_LogFunc;
	g_LogFunc = logFunc;
	return old;
}

int zRender::log_e(const std::wstring& tag, const std::wstring& msg, int rate/*=0*/, const std::wstring& key/*=L""*/)
{
	if(g_LogFunc)
	{
		g_LogFunc(tag, msg, rate, key);
		return 0;
	}
	else
	{
		return -1;
	}
}