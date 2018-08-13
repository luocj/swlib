/*
 * base_connection.cpp
 *
 *  Created on: 2016-3-2
 *      Author: luochj
 */

#include "base_connection.h"
#include "base_core_packet.h"
#include "base_server_info.h"

BASE_NAMESPACE_BEGIN_DECL

CConnection::CConnection()
{
	m_Fd = 0;
	m_Bev = NULL;
	Init();
}

CConnection::~CConnection()
{
	Init();
}

void CConnection::Init()
{
	m_Fd = 0;
	m_ServerType = enmServerType_Nothing;
	m_ServerID = 0;
	m_RemoteAddr.set_ip(0);
	m_RemoteAddr.set_port(0);
	if(m_Bev)
	{
		bufferevent_free(m_Bev);
	}
	m_Bev = NULL;
	m_Buffer.reset();
	//释放太多的内存占用
	m_Istrm.rewind(true);
	m_Istrm.reduce();
	m_packSize.rewind(true);
	m_packSize.reduce();
}

void CConnection::Read(struct bufferevent *bev, void *ctx)
{
	CConnection* conn = (CConnection*)ctx;
	if(!conn)
	{
		CORE_ERROR("conn read error : conn is NULL " );
		return;
	}
	conn->RecvSplitData();
}

void CConnection::Event(struct bufferevent *bev, short events, void *ctx)
{
    if (events & BEV_EVENT_ERROR)
    {
		//CConnection* conn = (CConnection*)ctx;
    	//CORE_DEBUG("conn: " << conn->GetInetAddr().to_string() << " recv error: " << events);
    }
	//连接关闭
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR))
    {
    	CConnection* conn = (CConnection*)ctx;
		if (conn != NULL)
		{
			conn->DisConnect();
		}
		else
		{
			CORE_ERROR("ctx to conn error!");
		}

    }
    //主动发起连接的完成连接
    if (events & BEV_EVENT_CONNECTED)
    {
    	CConnection* conn = (CConnection*)ctx;
		if (conn != NULL)
		{
			bufferevent_enable(bev, EV_READ | EV_WRITE);
			CORE_DEBUG("conn: " << conn->GetInetAddr().to_string() << " connected !! ");
		} 
		else
		{
			CORE_ERROR("ctx to conn error!");
		}
    	
    }
}

int32_t CConnection::Connect(event_base * base, const Inet_Addr& addr)
{
	if(!base)
	{
		CORE_ERROR("event base is NULL!");
		return -1;
	}
	m_Bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE | BEV_OPT_DEFER_CALLBACKS);
	if(!m_Bev)
	{
		CORE_ERROR("new buffer event  is NULL!");
		return ERROR__BEV_NULL;
	}
	m_RemoteAddr = addr;
	//异步连接
	int32_t ret = bufferevent_socket_connect(m_Bev, (struct sockaddr*)addr.get_addr(), sizeof(addr.ipaddr()));
	if (ret < 0 )
	{
		CORE_ERROR("connect " << addr.to_string() << " error !"  );
		return ret;
	}
	m_Fd = bufferevent_getfd(m_Bev);
	bufferevent_setcb(m_Bev, CConnection::Read, NULL, CConnection::Event, this);
	bufferevent_enable(m_Bev, EV_READ);
	return S_OK;
}

int32_t CConnection::DisConnect()
{
	CORE_DEBUG("conn: " << m_RemoteAddr.to_string() << " closeed!! ");
	CONNECT_MGR()->DelConn(m_ServerType, m_ServerID);
	Init();
	m_ConnPool.push_obj(this);
	return S_OK;
}

int32_t CConnection::Send(const void *data, size_t size)
{
	if(m_Bev)
	{
		CORE_DEBUG("send to : " << m_RemoteAddr.to_string() << " pack size " << size);
		SERVER_INFO()->nSendBitCount += size;
		return bufferevent_write(m_Bev, data, size);
	}
	return ERROR__BEV_NULL;
}
int32_t CConnection::Send(CBasePacket & pack)
{
	GAIN_BINSTREAM(binTemp);
	pack.Pack(*binTemp);
	PackProtocol packPro;
	packPro.msg = pack.msg_id_;
	packPro.headlen = sizeof(PackProtocol);			//头的长度
	packPro.datalen = binTemp->data_size();			//消息的长度
													//加入协议头
	GAIN_BINSTREAM(bin);
	packPro.Pack(*bin);
	RETURN_BINSTREAM(binTemp);
	//加入消息
	pack.Pack(*bin);

	int32_t ret = Send(bin->get_data_ptr(), bin->data_size());
	RETURN_BINSTREAM(bin);
	return ret;
}
void CConnection::RecvSplitData()
{
	if(!m_Bev)
	{
		return;
	}
	//libevent 接收队列中的数据
	uint32_t bevBuffSize = (uint32_t)evbuffer_get_length(bufferevent_get_input(m_Bev));
	if(bevBuffSize <= 0)
	{
		return;
	}
	//扩大TCP接收缓冲区,防止缓冲区太小造成收包异常
	if(m_Buffer.remaining_length() <= bevBuffSize)
	{
		if(m_Buffer.length() < MAX_BUFFER_SIZE)
		{
			m_Buffer.realloc_buffer(m_Buffer.length() - 1);
		}
		else
		{
			CORE_ERROR("conn buffer full : conn is " << m_RemoteAddr.to_string() << "  buf size :" << m_Buffer.length() );
			return;
		}
	}
	//接收数据
	uint32_t rc = 0;
	do
	{
		rc += m_Buffer.recv(m_Bev);
		if(rc > 0)
		{
			//统计接收到的数据长度
			SERVER_INFO()->nRecvBitCount += rc;
			//解析数据
			bool split_flag = true;
			while(split_flag)
			{
				int32_t split_ret = m_Buffer.split(m_Istrm);
				if(split_ret == 0)
				{
					split_flag = true;
					//todo 进行一个包的解析投递
					CBasePacket pack;
					try
					{
						pack.UnPack(m_Istrm);
					}
					catch (...)
					{
						CORE_ERROR("conn is " << m_RemoteAddr.to_string() << " split message error  !");
						DisConnect();
						return;
					}
					MsgDispatch(pack);
				}
				else if(split_ret < 0)
				{
					CORE_ERROR("conn is " << m_RemoteAddr.to_string() << " split message error  !" );
					return;
				}
				else
				{
					split_flag = false;
				}
			}
		}

	}while(rc < bevBuffSize);
}
void CConnection::SendPing()
{
	if (m_ServerType != enmServerType_Nothing && m_ServerID != 0)
	{
		INIT_BASE_PACK(ping, enmMsgType_Ping, 0, 0);
		CONNECT_MGR()->SendToServer(m_ServerType, m_ServerID, ping);
	}
}
int32_t CConnection::LSend(table msgHead, const char * data,int32_t nLength)
{
	uint32_t nSessionID = msgHead.get<int>("sessionID");
	uint32_t nMsgType = msgHead.get<int>("msgType");
	uint32_t nMsgID = msgHead.get<int>("msgID");

	INIT_BASE_PACK(pack, nMsgType, nMsgID, nSessionID);
	if (data != NULL)
	{
		pack.data_.assign(data, nLength);
	}
	return Send(pack);
}

int32_t CConnection::LSendByMsgID(uint32_t msgID, const char * data, int32_t nLength)
{
	INIT_BASE_PACK(pack,enmMsgType_Noti, msgID, 0);
	if (data != NULL)
	{
		pack.data_.assign(data, nLength);
	}
	return Send(pack);
}

void BASEOBJECT::CConnection::LSetTypeID(ServerType serverType, uint32_t serverID)
{
	m_ServerType = serverType;
	m_ServerID = serverID;
	CONNECT_MGR()->AddConn(serverType, serverID, this);
}

bool CConnection::MsgDispatch(CBasePacket & pack)
{
	if (pack.msg_type_ == enmMsgType_Ping)
	{
		m_ServerType = pack.server_type_;
		m_ServerID = pack.server_id_;
		CONNECT_MGR()->AddConn(pack.server_type_,pack.server_id_,this);
		CORE_DEBUG("conn : "<< (uint32_t)m_ServerType<<" : "<< m_ServerID <<" : "<< m_RemoteAddr.to_string() << " connect ok! " );
		return true;
	}
	//其他消息投递到lua处理
	pack.Print();

	table msgHead(LUA()->m_L);
	msgHead.set("serverType", pack.server_type_);
	msgHead.set("serverID", pack.server_id_);
	msgHead.set("sessionID", pack.session_id_);
	msgHead.set("msgType", pack.msg_type_);
	msgHead.set("msgID", pack.msg_id_);

	bool ret = lua_tinker::call<bool>(LUA()->m_L, "MsgDispatch", msgHead, pack.data_.c_str(), this);
	return ret;
}
BASE_NAMESPACE_END_DECL
