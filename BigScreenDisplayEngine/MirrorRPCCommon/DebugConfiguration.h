/*
 *	@file		DebugConfiguration.h
 *	@data		2013-11-19 15:25
 *	@author		zhu qing quan
 *	@brief		打开关闭输出调试信息的配置内容
 *  @Copyright	(C) 2013, by Ragile Corporation
 */

#pragma once
#ifndef _SOA_MIRROR_TOOLS_DEBUGCONFIGURATION_H_
#define _SOA_MIRROR_TOOLS_DEBUGCONFIGURATION_H_

#include <memory>
#include "Windows.h"

namespace SOA
{
namespace Mirror
{
namespace Tools
{
	class DebugConfiguration
	{
	public:
		inline void recordOn() {InterlockedExchange(&m_needRecord, 1);}
		inline void recordOff() {InterlockedExchange(&m_needRecord, 0);}
		inline bool needRecord() const {return InterlockedCompareExchange(const_cast<long*>(&m_needRecord), m_needRecord, m_needRecord)==1;}

		static DebugConfiguration* Instance()
		{
			if(!m_instance.get())
				m_instance = std::auto_ptr<DebugConfiguration>(new DebugConfiguration);
			return m_instance.get();
		}

		~DebugConfiguration() {};
	private:
		long m_needRecord;
		DebugConfiguration()
			: m_needRecord(0)
		{
		}
		static std::auto_ptr<DebugConfiguration> m_instance;
	};

	std::auto_ptr<DebugConfiguration> DebugConfiguration::m_instance(NULL);

	class DebugConfiguration_ref
	{
	public:
		DebugConfiguration_ref()
		{
			if(m_refCount==0)
				m_dbCfg = DebugConfiguration::Instance();
			InterlockedIncrement(&m_refCount);
		}
		~DebugConfiguration_ref()
		{
			InterlockedDecrement(&m_refCount);
		}

		inline void recordOn() { m_dbCfg->recordOn(); }
		inline void recordOff() { m_dbCfg->recordOff(); }
		inline bool needRecord() { return m_dbCfg->needRecord(); }
	private:
		DebugConfiguration* m_dbCfg;
		static long m_refCount;
	};

	long DebugConfiguration_ref::m_refCount = 0;

	extern DebugConfiguration_ref DebugCfg;
}
}
}

#endif // _SOA_MIRROR_TOOLS_DEBUGCONFIGURATION_H_