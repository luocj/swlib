/*
 * base_listener.cpp
 *
 *  Created on: 2016-3-2
 *      Author: Administrator
 */

#include "base_listener.h"
#include "base_connection_mgr.h"

BASE_NAMESPACE_BEGIN_DECL

int32_t CListener::Open(event_base *base,uint16_t port)
{
	m_InetAddr.set_port(port);
	if( base == NULL )
	{
		return -1;
	}
	m_Listener = evconnlistener_new_bind(base, CListener::AcceptCb, NULL, LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,(struct sockaddr*)m_InetAddr.get_addr(), sizeof(m_InetAddr.ipaddr()));
	//判断是否创建成功
	if( m_Listener == NULL )
	{
		return -1;
	}
	return 0;
}

void CListener::AcceptCb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx)
{
	Inet_Addr add(*((struct sockaddr_in*)address));
	CORE_INFO("accept " << add.to_string());
	struct event_base *base = evconnlistener_get_base(listener);
	struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE | BEV_OPT_DEFER_CALLBACKS);
	//从连接池中获取连接
	CConnection* conn = m_ConnPool.pop_obj();
	if (conn == NULL)
	{
		CORE_ERROR("accept get conn error!");
		return;
	}
	conn->SetBev(bev);
	conn->SetFd(fd);
	conn->GetInetAddr() = *((struct sockaddr_in*)address);
	//添加连接的监听
	bufferevent_setcb(bev, CConnection::Read, NULL, CConnection::Event, conn);
	//启用该bufevent写和读
	bufferevent_enable(bev, EV_READ | EV_WRITE);
}

void CListener::CloseListener()
{
	if(m_Listener)
	{
		evconnlistener_free(m_Listener);
		m_Listener = NULL;
	}
}
BASE_NAMESPACE_END_DECL
