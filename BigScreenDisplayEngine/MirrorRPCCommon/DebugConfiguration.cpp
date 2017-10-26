#include "DebugConfiguration.h"

using namespace SOA::Mirror::Tools;

long DebugConfiguration_ref::m_refCount = 0;

std::auto_ptr<DebugConfiguration> DebugConfiguration::m_instance(NULL);

DebugConfiguration* DebugConfiguration::Instance()
{
	if(!m_instance.get())
		m_instance = std::auto_ptr<DebugConfiguration>(new DebugConfiguration);
	return m_instance.get();
}

SOA::Mirror::Tools::DebugConfiguration::DebugConfiguration()
: m_needRecord(0)
{

}

SOA::Mirror::Tools::DebugConfiguration::~DebugConfiguration()
{

}

// void SOA::Mirror::Tools::DebugConfiguration::recordOn()
// {
// 	InterlockedExchange(&m_needRecord, 1);
// }
// 
// void SOA::Mirror::Tools::DebugConfiguration::recordOff()
// {
// 	InterlockedExchange(&m_needRecord, 0);
// }
// 
// bool SOA::Mirror::Tools::DebugConfiguration::needRecord() const
// {
// 	return InterlockedCompareExchange(const_cast<long*>(&m_needRecord), m_needRecord, m_needRecord)==1;
// }
