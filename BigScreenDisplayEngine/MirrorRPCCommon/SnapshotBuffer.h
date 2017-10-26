/*
 *	@file		SnapshotBuffer.h
 *	@data		2013-1-11 9:43
 *	@author		zhu qing quan
 *	@brief		Snapshot数据块。在此的实现中，数据块使用Windows的共享内存实现。
 *  @Copyright	(C) 2012, by Ragile Corporation
 */

#pragma once
#ifndef _SNAPSHOT_BUFFER_H_
#define _SNAPSHOT_BUFFER_H_

#include "MirrorTypes.h"
#include <Windows.h>

namespace SOA
{
namespace Mirror
{
namespace RPC
{
	struct SnapshotBuffer
	{
		size_t size;
		char   addr[64];
		void*  handle;

		ULONGLONG timestamp;
		int x;	//窗格在Screen中的位置。
		int y;	//窗格在Screen中的位置。
		SOA::Mirror::PIXFormat pixFormat;	//快照的数据格式。
		int width;	//窗格的分辨率
		int height;	//窗格的分辨率

		char* lockData();
		void unlockData();

	private:
		void* openHandle;
		void* pData;
	};

	void SetSnapshotBufferInit(SnapshotBuffer* buffer, size_t size, void* handle, const char* name);

	bool InitSnapshotBuffer(SnapshotBuffer* buffer, const char* shareMemoryName, size_t size, const char* data = NULL);

	void ReleaseSnapshotBuffer(SnapshotBuffer* buffer);
}
}
}

#endif // _SNAPSHOT_BUFFER_H_