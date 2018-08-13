/*
 * base_listener_thread.cpp
 *	use libevent-2.0.22
 *  Created on: 2016-3-3
 *      Author: luochj
 */

#include "base_net_thread.h"

BASE_NAMESPACE_BEGIN_DECL


CNetThread::CNetThread()
{
	set_thread_name("./net_thread");
#ifdef WIN32
	evthread_use_windows_threads();
	struct event_config *ec = event_config_new();
	event_config_set_flag(ec,EVENT_BASE_FLAG_STARTUP_IOCP);
	m_EventBase = event_base_new_with_config(ec);
	event_config_free(ec);
#else
	evthread_use_pthreads();
	m_EventBase = event_base_new();
#endif
}

CNetThread::~CNetThread()
{
	if(m_EventBase)
	{
		event_base_free(m_EventBase);
		m_EventBase = NULL;
	}
}

int32_t CNetThread::terminate()
{
	if(terminated_)
	{
		return 0;
	}
	terminated_ = true;
	if(m_EventBase)
	{
		event_base_loopexit(m_EventBase,NULL);
	}
#ifdef WIN32
	if (thr_handle_ != NULL)
	{
		WaitForSingleObject(thr_handle_, time_delay_);
	}
#else
	if (thr_handle_ != 0)
	{
		pthread_join(thr_handle_, NULL);
	}
#endif
	return 0;
}

void CNetThread::execute()
{
	if (!m_EventBase)
	{
		CORE_ERROR("event base is NULL!");
		return;
	}
	CORE_DEBUG("current event_base support method: "<< event_base_get_method(m_EventBase));
	int32_t ret = 1;
	while(true && !terminated_ && (ret >= 1))
	{
		//ret =  event_base_loop(m_EventBase, EVLOOP_ONCE | EVLOOP_NONBLOCK);
		ret =  event_base_dispatch(m_EventBase);
	}
	m_Listener.CloseListener();
	m_Http.CloseHttp();
	event_base_free(m_EventBase);
	m_EventBase = NULL;
	CORE_DEBUG("net thread end! terminated_: " << terminated_<<  " ret: "<< ret <<endl);
}

BASE_NAMESPACE_END_DECL

