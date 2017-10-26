#include "SnapshotBuffer.h"
#include <stdio.h>

using namespace SOA::Mirror::RPC;

void SOA::Mirror::RPC::SetSnapshotBufferInit(SnapshotBuffer* buffer, size_t size, void* handle, const char* name)
{
	memset(buffer, 0, sizeof(SnapshotBuffer));
	buffer->size = size;
	if(name!=NULL)
		sprintf(buffer->addr, "%s", name);
	buffer->handle = handle;
	buffer->timestamp = -1;
}

bool SOA::Mirror::RPC::InitSnapshotBuffer(SnapshotBuffer* buffer, const char* shareMemoryName, size_t size, const char* data)
{
	if(buffer==NULL || shareMemoryName==NULL || strlen(shareMemoryName)<=0 || size==0)
		return false;
	void* pHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, shareMemoryName);
	if(pHandle==NULL)
		return false;
	void *pMapView = MapViewOfFile(pHandle, FILE_MAP_ALL_ACCESS, 0, 0, size);
	if (pMapView == NULL)
		return false;
	if(data!=NULL)
		memcpy(pMapView, data, size);
	UnmapViewOfFile(pMapView);
	SOA::Mirror::RPC::SetSnapshotBufferInit(buffer, size, pHandle, shareMemoryName);		

	return true;
}

void SOA::Mirror::RPC::ReleaseSnapshotBuffer(SnapshotBuffer* buffer)
{
	if(buffer==NULL)
		return;
	if(buffer->handle!=NULL)
		CloseHandle(buffer->handle);
	buffer->handle = NULL;
	SOA::Mirror::RPC::SetSnapshotBufferInit(buffer, 0, NULL, NULL);
}

char* SOA::Mirror::RPC::SnapshotBuffer::lockData()
{
	if(size<=0 || strlen(addr)<=0)
		return NULL;
	if(openHandle==NULL)
	{
		void* tmephandle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, addr);
		if(tmephandle==NULL)
			return NULL;
		openHandle = tmephandle;
	}
	if(pData!=NULL)
		return NULL;
	void* pMapView = MapViewOfFile(openHandle, FILE_MAP_ALL_ACCESS, 0, 0, size);
	if (pMapView == NULL)
	{
		CloseHandle(openHandle);
		openHandle = NULL;
		return NULL;
	}
	pData = pMapView;
	return (char*)pMapView;
}

void SOA::Mirror::RPC::SnapshotBuffer::unlockData( )
{
	if(pData==NULL)
		return;
	UnmapViewOfFile(pData);
	pData = NULL;
	if(openHandle==NULL)
		return;
	CloseHandle(openHandle);
	openHandle = NULL;
}
