/*
 * base_listener_thread.h
 *
 *	todo 监听线程
 *  Created on: 2016-3-2
 *      Author: luochj
 */
#ifndef BASE_NET_THREAD_H_
#define BASE_NET_THREAD_H_
#include "base_namespace.h"
#include "base_thread.h"
#include "base_listener.h"
#include "base_http.h"
#include "base_connection.h"
#include "base_singleton.h"
#include "base_guard.h"
#include "base_thread_mutex.h"
#include "base_log_macro.h"
#include <event2/thread.h>

BASE_NAMESPACE_BEGIN_DECL

class CNetThread:public CThread
{
public:
	CNetThread();
	~CNetThread();
	virtual int32_t	terminate();
	//线程的入口处理函数
	virtual void execute();
	//退出线程
	void stop(){terminate();};

	void OpenListener(uint16_t port)
	{
		m_Listener.Open(m_EventBase,port);
	}

	//打开http监听
	void OpenHttp(uint16_t port)
	{
		m_Http.Open(m_EventBase, port);
	}

	event_base*	GetEvBase()
	{
		return m_EventBase;
	}

private:
	struct event_base *    m_EventBase;								//libevent的base
	CListener			   m_Listener;								//线程的监听
	CHttp				   m_Http;									//http服务
};

#define NET_THREAD_CREATE			CSingleton<CNetThread>::instance
#define NET_THREAD_INSTANCE			CSingleton<CNetThread>::instance
#define NET_THREAD_DESTROY			CSingleton<CNetThread>::destroy

BASE_NAMESPACE_END_DECL
#endif /* BASE_NET_THREAD_H_ */
