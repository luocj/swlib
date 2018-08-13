/*
 * base_listenner.h
 *
 *	todo ： http请求的处理
 *  Created on: 2016-5-13
 *      Author: luochj
 */

#ifndef BASE_HTTP_H_
#define BASE_HTTP_H_

#include "base_namespace.h"
#include  "base_inet_addr.h"
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/http_compat.h>


BASE_NAMESPACE_BEGIN_DECL

class CHttp
{
public:
	CHttp()
	{
		m_Evhttp = NULL;
		m_Buf = NULL;
	}
	~CHttp()
	{
	}
public:
	//回调函数
	static void RootHander(struct evhttp_request *req, void *arg);
	static void GenericHander(struct evhttp_request *req, void *arg);
	// 打开监听
	int32_t  Open(event_base *base,uint16_t port);
	// 关闭监听
	void CloseHttp();
private:
	evhttp*							m_Evhttp;
	Inet_Addr						m_InetAddr;
	struct evbuffer*				m_Buf;
};

BASE_NAMESPACE_END_DECL

#endif /* BASE_HTTP_H_ */
