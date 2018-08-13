/*
 * base_listenner.h
 *
 *	todo ： 监听类
 *  Created on: 2016-3-2
 *      Author: luochj
 */

#ifndef BASE_LISTENNER_H_
#define BASE_LISTENNER_H_

#include "base_namespace.h"
#include  "base_inet_addr.h"
#include <event2/listener.h>
#include <event2/bufferevent.h>


BASE_NAMESPACE_BEGIN_DECL

class CListener
{
public:
	CListener()
	{
		m_Listener = NULL;
	}
	~CListener()
	{
		if(m_Listener)
		{
			evconnlistener_free(m_Listener);
			m_Listener = NULL;
		}
	}
public:
	//监听回调函数
	static void AcceptCb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx);
	// 打开监听
	int32_t  Open(event_base *base,uint16_t port);
	// 关闭监听
	void CloseListener();
private:
	struct evconnlistener *			m_Listener;							//监听对象
	Inet_Addr						m_InetAddr;
};

BASE_NAMESPACE_END_DECL

#endif /* BASE_LISTENNER_H_ */
