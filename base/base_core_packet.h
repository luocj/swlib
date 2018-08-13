/****************************************************
*filename:	base_core_packet.h
*Create on: 2016-05
*Author:	luochj
*check list:
*************************************************************************************/
#ifndef BASE_CORE_PACKET_H
#define BASE_CORE_PACKET_H

#include "base_bin_stream.h"
#include "base_object_pool.h"
#include "base_def.h"
#include "base_log_macro.h"
#include "base_server_info.h"

#include <string>
using namespace std;

BASE_NAMESPACE_BEGIN_DECL

extern ObjectPool<BinStream, STREAM_POOL_SIZE>	STREAMPOOL;

#define GAIN_BINSTREAM(x) \
	BinStream* x = STREAMPOOL.pop_obj();\
	x->rewind(true)


#define RETURN_BINSTREAM(x) \
	if(x != NULL) \
	{\
		x->rewind(true);\
		x->reduce();\
		STREAMPOOL.push_obj(x);\
	}
//定义净荷相对大的报文，主要描述
class CBasePacket
{
public:
	CBasePacket()
	{
		server_id_		= 0;
		server_type_	= enmServerType_Nothing;
		msg_id_			= 0;
		msg_type_		= enmMsgType_Req;
		data_			= "";
		session_id_     = -1;
	};

	~CBasePacket()
	{

	}

	CBasePacket& operator=(const CBasePacket& packet)
	{
		server_id_		= packet.server_id_;
		server_type_	= packet.server_type_;
		msg_id_			= packet.msg_id_;
		msg_type_		= packet.msg_type_;
		data_			= packet.data_;
		session_id_     = packet.session_id_;

		return *this;
	}

	//编码解码函数
	 void	Pack(BinStream& strm) const;
	
	//解码函数
	 void	UnPack(BinStream& strm);

	 void	Print() const
	{
		CORE_DEBUG( "Core Packet, {" << "server_id = " << server_id_ 
			<<", server_type = " << (uint16_t)server_type_ 
			<<", msg_id_ = " << msg_id_ 
			<< ", msg_type = " << (uint16_t) msg_type_
			<<", session_id_ = " << (int32_t) session_id_
			<< ", data_ = "  << data_ 
			<<"}" << std::endl);
	}
	
public:
	uint32_t			server_id_;		//服务器ID
	uint8_t				server_type_;	//服务器类型,0表示客户
	uint32_t			msg_id_;		//消息ID
	MsgType				msg_type_;		//消息类型，例如独立的PING PONG消息，握手消息，应用层消息等
	int32_t     		session_id_;    //用户需要回应的请求设置会话id
	string				data_;			//消息内容
	
};

// 数据协议标记头
#define PACK_HEAD_TAG_0     'S'
#define PACK_HEAD_TAG_1     'W'

// 基本数据包协议
class PackProtocol
{
public:
	PackProtocol()
	{
		tag[0] = PACK_HEAD_TAG_0;
		tag[1] = PACK_HEAD_TAG_1;
		data1 = 0;
	};

	~PackProtocol()
	{

	}
	//编码解码函数
	void	Pack(BinStream& strm) const;

	//解码函数
	void	UnPack(BinStream& strm);

public:
	char    tag[2];				 // 标记
	uint16_t  msg;                // 消息编号
	union
	{
		struct
		{
			uint32_t  headlen : 6;        // 包头长度（某些情况下可能需要扩展包头，因此包头的数据长度不一定就是sizeof(PackProtocol)）
			uint32_t  datalen : 24;       // 数据包长度（不包括包头）
			uint32_t  format : 1;         // 数据格式（0 - json，1 － 二进制）
			uint32_t  compress : 1;       // 是否被压缩（0 － 未压缩，1 － zip压缩。暂未使用）
		};
		uint32_t data1;
	};
};

#define INIT_BASE_PACK(p, MSGTYPE, MSGID, SESSIONID)\
	CBasePacket p;\
	p.msg_id_ = MSGID;\
	p.msg_type_ = MSGTYPE;\
	p.session_id_ = SESSIONID;\
	p.server_type_ = SERVER_INFO()->nServerType;\
	p.server_id_ = SERVER_INFO()->nServerID;

BASE_NAMESPACE_END_DECL

#endif
/************************************************************************************/

