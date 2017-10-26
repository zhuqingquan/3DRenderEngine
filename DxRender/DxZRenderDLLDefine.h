#pragma once
#ifndef _ZRENDER_DXRENDER_DLL_DEFINE_H_
#define _ZRENDER_DXRENDER_DLL_DEFINE_H_

#ifdef _WINDOWS

#ifdef DX_ZRENDER_DLL_RXPORT
#define DX_ZRENDER_EXPORT_IMPORT _declspec(dllexport)
#else
#define DX_ZRENDER_EXPORT_IMPORT _declspec(dllimport)
#endif

#endif //_WINDOWS

#endif //_ZRENDER_DXRENDER_DLL_DEFINE_H_