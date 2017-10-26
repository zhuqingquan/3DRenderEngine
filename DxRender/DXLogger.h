/**
 *	@date		2017:4:13   16:14
 *	@name	 	DXLogger.h
 *	@author		zhuqingquan	
 *	@brief		logger for DxRender module
 **/

#ifndef _DX_RENDER_LOGGER_H_
#define _DX_RENDER_LOGGER_H_

#include <string>
#include "DxZRenderDLLDefine.h"

namespace zRender
{
	typedef void (*DxLogFunc)(const std::wstring& tag, const std::wstring& msg, int rate, const std::wstring& key);
	/**
	 *	@name			setDxLogFunc
	 *	@brief			Use this func to set the function that is used to do the real log work.
	 *	@param[in]		DxLogFunc logFunc log function
	 *	@return			DxLogFunc return the old log function
	 **/
	DxLogFunc DX_ZRENDER_EXPORT_IMPORT setDxLogFunc(DxLogFunc logFunc);

	/**
	 *	@name			log_e
	 *	@brief			log error
	 *	@param[in]		const wstring & tag
	 *	@param[in]		const wstring & msg 
	 *	@return			int 0--success others--failed
	 **/
	int log_e(const std::wstring& tag, const std::wstring& msg, int rate=0, const std::wstring& key=L"");
}

#endif//_DX_RENDER_LOGGER_H_