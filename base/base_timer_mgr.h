/*
 * base_timer_mgr.h
 *
 *	todo ：基于libevent timer 的定时器管理
 *  Created on: 2016-3-31
 *      Author: luochj
 */

#ifndef BASE_TIMER_MGR_H_
#define BASE_TIMER_MGR_H_

#include "base_namespace.h"
#include "base_singleton.h"
#include "base_def.h"
#include "base_object_pool.h"
#include "base_thread_mutex.h"
#include "base_typedef.h"
#include "base_log_macro.h"
#include <map>
#include <event2/event.h>  

using namespace std;
BASE_NAMESPACE_BEGIN_DECL

struct Timer;
class CTimerMgr;
typedef ObjectMutexPool<Timer, BaseThreadMutex, TIMER_POOL_SIZE>	Timer_Pool;
extern Timer_Pool	TIMER_POOL;

class CTimerHandler
{
public:
	CTimerHandler() {}
	virtual ~CTimerHandler() {}
	//提供给上层的回调函数
	virtual int32_t Timeout(const void *pData, uint32_t nTimerID) = 0;
};

struct Timer
{
	uint32_t			timer_id;
	struct event*		ev;
	struct timeval		tv;
	void*				data_;	//timer带的数据
	CTimerHandler*		cback;
	bool				once;
	CTimerMgr*			tmgr;
	Timer()
	{
		Init();
	}
	void Init()
	{
		timer_id = 0;
		ev = NULL;
		tv = { 0, 0 };
		data_ = NULL;
		cback = NULL;
		once = true;
		tmgr = NULL;
	}
};

typedef map<uint32_t, Timer*> TimerMap;

class CTimerMgr
{
public:
	CTimerMgr();
	~CTimerMgr();
	
	static void timeout_cb(int fd, short event, void *params);

	void Init(struct event_base* pEvent);
	//创建定时器
	//nTime豪秒
	uint32_t CreateTimer(const float nTime, CTimerHandler* pCback,void* pData = NULL,const bool bOnce = true);

	void DelTimer(const uint32_t nTimerID);

private:
	uint32_t GetTimerID()
	{
		static uint32_t nTimerID = 0;
		if (nTimerID >= 0x0ffffff)
		{
			nTimerID = 0;
		}
		return ++nTimerID;
	}
private:
	struct event_base*    m_EventBase;								//libevent的base
	TimerMap			  m_TimerMap;								//管理上层创建出来的定时器
	BaseThreadMutex		  m_Lock;									//线程锁
};

#define TIMER_MGR_CREATE			CSingleton<CTimerMgr>::instance
#define TIMER_MGR					CSingleton<CTimerMgr>::instance
#define TIMER_MGR_DESTROY			CSingleton<CTimerMgr>::destroy

BASE_NAMESPACE_END_DECL

#endif /* BASE_TIMER_MGR_H_ */
