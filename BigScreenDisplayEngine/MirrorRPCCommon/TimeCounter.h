#pragma once
#ifndef _SOA_MIRROR_RPC_TIME_COUNTER_H_
#define _SOA_MIRROR_RPC_TIME_COUNTER_H_

#include <Windows.h>

namespace SOA
{
namespace Mirror
{
namespace RPC
{
	class TimeCounter
	{
	public:
		TimeCounter()
			: m_begin(0)
			, m_end(0)
			, m_lastHit(0)
		{
			::QueryPerformanceFrequency(&freq);
		}

		void begin()
		{
			::QueryPerformanceCounter(&systemTime);
			m_begin = systemTime.QuadPart * 1000 / freq.QuadPart;
		}

		void end()
		{
			::QueryPerformanceCounter(&systemTime);
			m_end = systemTime.QuadPart * 1000 / freq.QuadPart;
		}

		unsigned int hit()
		{
			::QueryPerformanceCounter(&systemTime);
			LONGLONG now = systemTime.QuadPart * 1000 / freq.QuadPart;
			if(m_lastHit==0)
			{
				m_lastHit = now;
				return 0;
			}
			else
			{
				unsigned int interval = now - m_lastHit;
				m_lastHit = now;
				return interval;
			}
		}

		LONGLONG now_in_millsec()
		{
			::QueryPerformanceCounter(&systemTime);
			LONGLONG now = systemTime.QuadPart * 1000 / freq.QuadPart;
			return now;
		}

		void outputSpend()
		{
			printf("spend: %d\n", m_end-m_begin);
		}
	private:
		LARGE_INTEGER freq;
		LARGE_INTEGER systemTime;
		LONGLONG m_begin;
		LONGLONG m_end;

		LONGLONG m_lastHit;
	};
}
}
}

#endif