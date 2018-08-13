/*
* uuid.h
* UUID的生成
*  Created on: 2016-7-14
*      Author: luochj
*/

#ifndef __MINDUMP_H
#define __MINDUMP_H


#include "base_def.h"
#include "base_namespace.h"
#include "base_os.h"
#include "base_date_time.h"
#include "base_singleton.h"

using namespace BASE_NAMEPSACE_DECL;

// --------------------------------------------------------------------------------------


class CUUID
{
public:
	CUUID()
	{
		m_Now = DateTime::GetNow().GetSeconds();
		m_TimeDev = DateTime(2016,7,1).GetSeconds();
		m_Index = 0;
	}
	~CUUID() 
	{

	}
public:
	uint64_t GetNewID(uint64_t nSpecialNum)
	{
		BASE_GUARD_RETURN(BaseThreadMutex, cf_mon, m_Lock, 0);
		uint64_t nNowTemp = DateTime::GetNow().GetSeconds();
		if (m_Now != nNowTemp)
		{
			m_Now = nNowTemp;
			m_Index = 0;
		}
		if (m_Index >= 0x000fffff)
		{
			usleep(1000 * 1000);
			m_Now = DateTime::GetNow().GetSeconds();
			m_Index = 0;
		}
		InterlockedIncrement(&m_Index);
		uint64_t ret = m_Now - m_TimeDev;
		ret = ret << 31;
		ret |= nSpecialNum << 20;
		ret |= m_Index;
		return ret;
	}
private:
	uint64_t				m_Now;		//现在的时间
	uint64_t				m_TimeDev;	//减去的时间
	uint32_t				m_Index;	//现在的Index
	BaseThreadMutex			m_Lock;		//线程锁
}; 

#define UUID						CSingleton<CUUID>::instance
#define DESTROY_UUID    			CSingleton<CUUID>::destroy

#endif //__MINDUMP_H


