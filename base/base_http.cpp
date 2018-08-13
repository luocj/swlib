/*
 * base_http.cpp
 *
 *  Created on: 2016-5-13
 *      Author: luocj
 */

#include "base_http.h"
#include "base_log_macro.h"

BASE_NAMESPACE_BEGIN_DECL

int32_t CHttp::Open(event_base * base, uint16_t port)
{
	m_InetAddr.set_port(port);
	if (base == NULL)
	{
		return ERROR_EVENT_BASE_NULL;
	}
	//创建http
	m_Evhttp = evhttp_new(base);
	if (m_Evhttp == NULL)
	{
		CORE_ERROR("create http error!");
		return ERROR_HTTP_NULL;
	}
	//绑定端口地址
	int32_t ret = evhttp_bind_socket(m_Evhttp, m_InetAddr.ip_to_string().c_str(), m_InetAddr.get_port());
	if (ret < 0)
	{
		CloseHttp();
		CORE_ERROR("create http bind socket error! ret :" << ret);
		return ERROR_HTTP_NULL;
	}
	m_Buf = evbuffer_new();
	if (m_Buf == NULL)
	{
		CloseHttp();
		CORE_ERROR("create http evbuffrt error!" );
		return ERROR_HTTP_NULL;
	}
	//添加回调函数
	ret = evhttp_set_cb(m_Evhttp,"/", RootHander, m_Buf);
	if (ret < -1)
	{
		CloseHttp();
		CORE_ERROR("set http call back error! ret :" << ret);
		return ERROR_HTTP_NULL;
	}
	evhttp_set_gencb(m_Evhttp, GenericHander, m_Buf);

	return S_OK;
}

void CHttp::CloseHttp()
{
	if (m_Evhttp)
	{
		evhttp_free(m_Evhttp);
		m_Evhttp = NULL;
	}
	if (m_Buf)
	{
		evbuffer_free(m_Buf);
		m_Buf = NULL;
	}
}

void CHttp::RootHander(struct evhttp_request *req, void *arg)
{
	struct evbuffer *buf = (evbuffer*)arg;
	if (buf == NULL)
	{
		CORE_ERROR("failed to create response buffer");
		return;
	}
    evbuffer_add_printf(buf, "Hello World!/n");
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
}
void CHttp::GenericHander(struct evhttp_request *req, void *arg)
{
	struct evbuffer *buf = (evbuffer*)arg;
	if (buf == NULL)
	{
		CORE_ERROR("failed to create response buffer");
		return;
	}
	string str = evhttp_request_uri(req);
    evbuffer_add_printf(buf, "Requested: %s/n", str.c_str());
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
}

BASE_NAMESPACE_END_DECL