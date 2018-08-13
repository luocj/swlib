/*
 * base_timer_mgr.cpp
 *
 *  Created on: 2016-3-31
 *      Author: luochj
 */

#include "base_timer_mgr.h"
#include "base_net_thread.h"

BASE_NAMESPACE_BEGIN_DECL
Timer_Pool	TIMER_POOL;

CTimerMgr::CTimerMgr()
{
	m_EventBase = NULL;
	m_TimerMap.clear();
}

CTimerMgr::~CTimerMgr()
{
	for (TimerMap::iterator it = m_TimerMap.begin(); it != m_TimerMap.end(); it++)
	{
		evtimer_del(it->second->ev);
		//回收Timer
		it->second->Init();
		TIMER_POOL.push_obj(it->second);
	}
	m_TimerMap.clear();
}

void CTimerMgr::Init(struct event_base* pEvent)
{
	m_EventBase = pEvent;
}

void CTimerMgr::timeout_cb(int fd, short event, void *params)
{
	Timer* pTimer = (Timer*)params;
	if (pTimer == NULL)
	{
		CORE_ERROR("timer error pTimer is NULL!!");
		return;
	}
	//调用timeout的回调函数
	if (pTimer->cback)
	{
		pTimer->cback->Timeout(pTimer->data_, pTimer->timer_id);
	}
	if (pTimer->once && pTimer->tmgr)
	{
		pTimer->tmgr->DelTimer(pTimer->timer_id);
	}
}

//创建定时器
/*
*	nTime  毫秒
*	pCback 回调
*	pData 数据
*	bOnce 是否只触发一次
*	return 定时器ID
*/
uint32_t CTimerMgr::CreateTimer(const float nTime, CTimerHandler* pCback, void* pData, const bool bOnce)
{
	Timer* pTimer = TIMER_POOL.pop_obj();
	if (pTimer == NULL)
	{
		CORE_ERROR("create timer error: can not get pTimer!!");
		return 0;
	}
	//创建定时器的事件
	BASE_GUARD_RETURN(BaseThreadMutex, cf_mon, m_Lock,0);
	if (!bOnce)
	{
		pTimer->ev = event_new(m_EventBase, -1, EV_PERSIST, CTimerMgr::timeout_cb, pTimer);
	}
	else
	{
		pTimer->ev = evtimer_new(m_EventBase, CTimerMgr::timeout_cb, pTimer);
	}
	if (pTimer->ev == NULL)
	{
		CORE_ERROR("create timer error: evtimer_new error!!");
		return 0;
	}
	pTimer->timer_id = GetTimerID();
	pTimer->tv.tv_sec = (long)(nTime / 1000.f);
	pTimer->tv.tv_usec = (long)((nTime - pTimer->tv.tv_sec *  1000) * 1000.f);
	pTimer->data_ = pData;
	pTimer->cback = pCback;
	pTimer->once = bOnce;
	pTimer->tmgr = this;
	//添加定时器到libevent
	evtimer_add(pTimer->ev, &(pTimer->tv));
	m_TimerMap[pTimer->timer_id] = pTimer;
	return pTimer->timer_id;
}

void CTimerMgr::DelTimer(const uint32_t nTimerID)
{
	BASE_GUARD(BaseThreadMutex, cf_mon, m_Lock);
	TimerMap::iterator it = m_TimerMap.find(nTimerID);
	if (it == m_TimerMap.end())
	{
		return ;
	}
	evtimer_del(it->second->ev);
	//回收Timer
	it->second->Init();
	TIMER_POOL.push_obj(it->second);
	m_TimerMap.erase(nTimerID);
}

BASE_NAMESPACE_END_DECL
