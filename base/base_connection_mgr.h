/*
 * base_connection_mgr.h
 *
 *  Created on: 2016-3-21
 *      Author: luochj
 */

#ifndef BASE_CONNECTION_MGR_H_
#define BASE_CONNECTION_MGR_H_

#include "base_namespace.h"
#include "base_singleton.h"
#include "base_object_pool.h"
#include "base_connection.h"
#include "base_net_thread.h"
#include "base_core_packet.h"

#include <map>
using namespace std;
BASE_NAMESPACE_BEGIN_DECL

//连接池
class CConnection;
typedef  ObjectPool<CConnection,2000> ConnPool;
extern ConnPool	m_ConnPool;

typedef map<uint32_t,  CConnection* > ServerConnMap;
typedef map<ServerType, ServerConnMap> ServerTypeMap;

class CConnectionMgr
{
public:
	CConnectionMgr();
	~CConnectionMgr();
public:
	//添加连接(此接口不连接，只是把已经接收的连接，加入管理器)
	//在连接完成注册的时候完成
	int32_t AddConn(const ServerType nServerType, const uint32_t nServerID,  CConnection* pConn);
	
	//删除连接
	int32_t DelConn(const ServerType nServerType, const uint32_t nServerID);

	//连接server
	int32_t ConnectServer(const ServerType nServerType, const uint32_t nServerID, const Inet_Addr& addr);

	//直接发送数据的接口
	int32_t SendToServer(const ServerType nServerType, const uint32_t nServerID, const void *data, size_t size);

	//发送带头的消息
	int32_t SendToServer(const ServerType nServerType, const uint32_t nServerID, CBasePacket& basePack);
	//广播
	int32_t SendToServer(const ServerType nServerType, CBasePacket& basePack);
	//获取连接
	CConnection* GetConn(const ServerType nServerType, const uint32_t nServerID);
	//获取连接的数量
	uint32_t GetConnCount(const ServerType nServerType);
private:
	ServerTypeMap m_ServerTypeMap;
};

#define CONNECT_MGR_CREATE			CSingleton<CConnectionMgr>::instance
#define CONNECT_MGR					CSingleton<CConnectionMgr>::instance
#define CONNECT_MGR_DESTROY			CSingleton<CConnectionMgr>::destroy

BASE_NAMESPACE_END_DECL

#endif /* BASE_CONNECTION_MGR_H_ */
