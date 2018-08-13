#include "base_core_packet.h"
//最大10M
#define MAX_PACKET_BODY_SIZE	1048576

BASE_NAMESPACE_BEGIN_DECL
ObjectPool<BinStream, STREAM_POOL_SIZE>	STREAMPOOL;

void CBasePacket::Pack(BinStream& strm) const
{
	strm << server_id_ << server_type_ << msg_id_ << msg_type_ << session_id_ << data_;
}

void CBasePacket::UnPack(BinStream& strm)
{
	strm >> server_id_ >> server_type_ >> msg_id_ >> msg_type_ >> session_id_ >> data_;
}


void PackProtocol::Pack(BinStream& strm) const
{
	strm << tag[0] << tag[1] << msg << data1;
}

void PackProtocol::UnPack(BinStream& strm)
{
	strm >> tag[0] >> tag[1] >> msg >> data1 ;
}

BASE_NAMESPACE_END_DECL
