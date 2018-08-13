/*
 * base_connection.h
 * 连接
 *  Created on: 2016-3-2
 *      Author: luochj
 */

#ifndef BASE_CONNECTION_H_
#define BASE_CONNECTION_H_

#include "base_namespace.h"
#include "base_inet_addr.h"
#include "base_def.h"
#include "base_log_macro.h"
#include "base_block_buffer.h"
#include "base_bin_stream.h"
#include "base_connection_mgr.h"
#include "base_core_packet.h"
#include "base_lua.h"

#include <event2/listener.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

BASE_NAMESPACE_BEGIN_DECL

//16K队列
typedef CReciverBuffer_T<BinStream, 1024 * 16>	 RBuffer;

class CConnection
{
public:
	CConnection();
	~CConnection();
	void Init();
	//连接读到数据的回调函数
	static void Read(struct bufferevent *bev, void *ctx);
	//事件回调函数
	static void Event(struct bufferevent *bev, short events, void *ctx);
	//主动发起连接
	int32_t Connect(event_base * base, const Inet_Addr& addr);
	int32_t DisConnect();
	//发送消息
	int32_t Send(const void *data, size_t size);
	//发送消息
	int32_t Send(CBasePacket& pack);
	//设置连接的地址
	Inet_Addr& GetInetAddr()
	{
		return m_RemoteAddr;
	}
	void SetFd(evutil_socket_t& fd)
	{
		m_Fd = fd;
	}
	void SetBev(bufferevent* bev)
	{
		m_Bev = bev;
	}
	//接收并拆分为数据包
	void RecvSplitData();

	void SendPing();

	//提供给lua发送信息
	int32_t LSend(table msgHead,const char *data, int32_t nLength);
	//（发送消息简单版）一般用于直接发送消息到玩家客户端的连接
	int32_t LSendByMsgID(uint32_t msgID, const char *data, int32_t nLength);
	//设置连接的信息
	void LSetTypeID(ServerType serverType, uint32_t serverID);

	ServerType GetType()
	{
		return m_ServerType;
	}
	uint32_t GetID()
	{
		return m_ServerID;
	}

	table LGetRemoteAddr()
	{
		table ret(LUA()->m_L);
		ret.set("ip", m_RemoteAddr.ip_to_string().c_str());
		ret.set("port",(int32_t)m_RemoteAddr.get_port());
		return ret;
	}

private:
	//消息分发
	bool MsgDispatch(CBasePacket& pack);

private:
	evutil_socket_t	 				m_Fd;					//套接字
	Inet_Addr						m_RemoteAddr;
	struct bufferevent*				m_Bev;					//eventBuf
	RBuffer							m_Buffer;				//接收BUFFER
	BinStream						m_Istrm;				//read函数中使用，防止每次申请，提高效率
	SizeBinStream					m_packSize;				//send函数中使用，防止每次申请，提高效率
public:
	//远端的serverType,serverID
	ServerType						m_ServerType;			//0表示客户端
	uint32_t						m_ServerID;				//端ID
};

BASE_NAMESPACE_END_DECL

#endif /* BASE_CONNECTION_H_ */
