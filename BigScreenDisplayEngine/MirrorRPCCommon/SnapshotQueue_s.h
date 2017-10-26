/*
 *	@file		SnapshotQueue_s.h
 *	@data		2013-1-10 16:40
 *	@author		zhu qing quan
 *	@brief		快照的队列，使用共享内存实现，读取线程安全
 *  @Copyright	(C) 2012, by Ragile Corporation
 */

#pragma once
#ifndef _SNAPSHOT_QUEUE_S_H_
#define _SNAPSHOT_QUEUE_S_H_

#include <Windows.h>

namespace SOA
{
namespace Mirror
{
namespace RPC
{
	template <typename T>
	class SnapshotQueue_s
	{
	public:
		typedef long size_t;

		bool push(const T& val);
		void pop();
		bool nextw(T& outVal);
		bool   front(T& elemt);

		bool initQueue(size_t initSize, const char* name);
		void release();
	private:
		size_t size;
		size_t writePos;
		size_t readPos;

		void* m_handle;
		void* m_mapView;
		char pContent[56];		
	};

	template <typename T>
	void* InitQueue(SnapshotQueue_s<T>*& queue, const char* name, typename SnapshotQueue_s<T>::size_t initSize, bool isCreate = true)
	{
		int size = sizeof(SnapshotQueue_s<T>);
		void* phandle = NULL;
		if(isCreate)
			phandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, name);
		else
			phandle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name);
		// 创建错误
		if ((phandle == NULL) || (phandle == INVALID_HANDLE_VALUE))
			return NULL;
		void *pMapView = MapViewOfFile(phandle, FILE_MAP_ALL_ACCESS, 0, 0, size);
		if (pMapView == NULL)
		{
			CloseHandle(phandle);
			phandle = NULL;
			return NULL;
		}
		SnapshotQueue_s<T>* tempQueue = (SnapshotQueue_s<T>*)pMapView;
		if(!isCreate)
			goto returnLabel;
		char contentName[56] = {0};
		sprintf(contentName, "%s_content", name);
		bool ret = tempQueue->initQueue(initSize, contentName);
		if(!ret)
		{
			if(phandle!=NULL)
			{
				CloseHandle(phandle);
				phandle = NULL;
			}
			return NULL;
		}
returnLabel:
		queue = tempQueue;
		return phandle;
	}

	template <typename T>
	void ReleaseQueue(SnapshotQueue_s<T>* queue, void* handle, bool isNeedRealse = true)
	{
		if(queue!=NULL)
		{
			if(isNeedRealse)
				queue->release();
			UnmapViewOfFile(queue);
		}
 		if(handle!=NULL)
 		{
 			CloseHandle(handle);
 			handle = NULL;
 		}
	}

	template <typename T>
	bool SnapshotQueue_s<T>::initQueue(size_t initSize, const char* name)
	{
		size_t 	buffeSize = initSize*(sizeof(T));
		m_handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, buffeSize, name);
		// 创建错误
		if ((m_handle == NULL) || (m_handle == INVALID_HANDLE_VALUE))
		{
			m_handle = NULL;
			return false;
		}
		//void *pMapView = MapViewOfFile(m_handle, FILE_MAP_ALL_ACCESS, 0, 0, size*sizeof(T));
		//if (pMapView == NULL)
		//{
		//	CloseHandle(m_handle);
		//	m_handle = NULL;
		//	return false;
		//}
		//m_mapView = pMapView;

		sprintf(pContent, "%s", name);
		size = initSize;
		readPos = -1;
		writePos = 0;	
		return true;
	}

	
	template <typename T>
	void SnapshotQueue_s<T>::release()
	{
		//if(m_mapView!=NULL)
		//{
		//	UnmapViewOfFile(m_mapView);
		//	m_mapView = NULL;
		//}
		if(this->m_handle!=NULL)
		{
			CloseHandle(this->m_handle);
			this->m_handle = NULL;
		}

		this->size = 0;
		this->readPos = -1;
		this->writePos = 0;
	}

	
	template <typename T>
	bool   SnapshotQueue_s<T>::nextw(T& outVal)
	{
		if(size==0)
			throw "Not initial";

		SnapshotQueue_s<T>::size_t rp;
		InterlockedExchange(&rp, readPos);
		if(((writePos==size) && (rp==-1)) || ((writePos%size)==rp))
			return false;

		SnapshotQueue_s<T>::size_t t = writePos;
		if(t==size)
			t = 0;

		void* tHandle = OpenFileMapping(FILE_MAP_READ, FALSE, pContent);
		if(tHandle==NULL)
			return false;
		void *pMapView = MapViewOfFile(tHandle, FILE_MAP_READ, 0, 0, size*sizeof(T));
		if (pMapView == NULL)
		{
			return false;
		}
		T* temp = (T*)pMapView;
		outVal = *(temp+t);

		UnmapViewOfFile(pMapView);
		CloseHandle(tHandle);
		return true;
	}
	template <typename T>
	bool   SnapshotQueue_s<T>::front(T& elemt)
	{
		if(size==0)
			throw "Not initial";

		SnapshotQueue_s<T>::size_t wp;
		InterlockedExchange(&wp, writePos);
		if(((readPos+1)%size)==wp || (wp==size)&&(readPos+1==size))
			return false;

		SnapshotQueue_s<T>::size_t t = readPos+1;
		if(t==size)
			t = 0;
		void* tHandle = OpenFileMapping(FILE_MAP_READ, FALSE, pContent);
		if(tHandle==NULL)
			return false;
		void *pMapView = MapViewOfFile(tHandle, FILE_MAP_READ, 0, 0, size*sizeof(T));
		if (pMapView == NULL)
		{
			return false;
		}
		T* pRead = (T*)pMapView;

		elemt = *(pRead+t);

		UnmapViewOfFile(pMapView);
		CloseHandle(tHandle);
		return true;
	}

	template <typename T>
	bool SnapshotQueue_s<T>::push(const T& val)
	{
		if(size==0)
			throw "Not initial";
		SnapshotQueue_s<T>::size_t rp;
		InterlockedExchange(&rp, readPos);
		if(((writePos==size) && (rp==-1)) || ((writePos%size)==rp))
			return false;

		SnapshotQueue_s<T>::size_t t = writePos;
		if(t==size)
			t = 0;

		void* tHandle = OpenFileMapping(FILE_MAP_WRITE, FALSE, pContent);
		if(tHandle==NULL)
			return false;
		void *pMapView = MapViewOfFile(tHandle, FILE_MAP_WRITE, 0, 0, size*sizeof(T));
		if (pMapView == NULL)
		{
			return false;
		}
		T* pWrite = (T*)pMapView;

		//memset(pWrite+t, 0, sizeof(T));
		*(pWrite+t) = val;

		UnmapViewOfFile(pMapView);
		CloseHandle(tHandle);

		++t;
		InterlockedExchange(&writePos, t);
		return true;
	}

	template <typename T>
	void SnapshotQueue_s<T>::pop()
	{
		if(size==0)
			return;

		SnapshotQueue_s<T>::size_t wp;
		InterlockedExchange(&wp, writePos);
		if(((readPos+1)%size)==wp || (wp==size)&&(readPos+1==size))
			return;
		SnapshotQueue_s<T>::size_t t = readPos+1;
		if(t==size)
			t = 0;

		void* tHandle = OpenFileMapping(FILE_MAP_READ, FALSE, pContent);
		if(tHandle==NULL)
			return;
		void *pMapView = MapViewOfFile(tHandle, FILE_MAP_READ, 0, 0, size*sizeof(T));
		if (pMapView == NULL)
		{
			return;
		}
		UnmapViewOfFile(pMapView);
		CloseHandle(tHandle);

		InterlockedExchange(&readPos, t);
	}
}
}
}

#endif // _SNAPSHOT_QUEUE_S_H_