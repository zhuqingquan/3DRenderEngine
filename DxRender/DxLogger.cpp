#include "DXLogger.h"
#include <stdarg.h>

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

int zRender::log(const std::wstring& tag, LogLevel level, const wchar_t* format, ...)
{
	wchar_t errmsg[1024] = { 0 };
	va_list argList;
	va_start(argList, format);
	swprintf_s(errmsg, 512, format, argList);
	log_e(tag, errmsg);
	va_end(argList);
	return 0;
}
