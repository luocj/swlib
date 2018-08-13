/*
 * base_connection_mgr.cpp
 *
 *  Created on: 2016-3-21
 *      Author: luochj
 */


#include "base_connection_mgr.h"
#include "base_lua.h"

BASE_NAMESPACE_BEGIN_DECL
ConnPool	m_ConnPool;

CConnectionMgr::CConnectionMgr()
{
	m_ServerTypeMap.clear();
}
CConnectionMgr::~CConnectionMgr()
{
	m_ServerTypeMap.clear();
}
int32_t CConnectionMgr::AddConn(const ServerType nServerType, const uint32_t nServerID, CConnection* pConn)
{
	if (nServerType == enmServerType_Nothing)
	{
		CORE_WARNING("add a error type server serverType :" << nServerType);
		return ERROR_SERVER_TYPE;
	}
	ServerTypeMap::iterator itType = m_ServerTypeMap.find(nServerType);
	if (itType == m_ServerTypeMap.end())
	{
		ServerConnMap commMap;
		commMap[nServerID] = pConn;
		m_ServerTypeMap[nServerType] = commMap;
	}
	else
	{
		itType->second[nServerID] = pConn;
	}
	CORE_DEBUG("add conn: " << (uint32_t)nServerType << ":" << nServerID << " addrs: " << pConn->GetInetAddr().to_string());
	//调用lua的添加连接回调
	lua_tinker::call<void>(LUA()->m_L, "AddConn", nServerType, nServerID);
	return S_OK;
}

int32_t CConnectionMgr::DelConn(const ServerType nServerType, const uint32_t nServerID)
{
	if (nServerType == enmServerType_Nothing)
	{
		return ERROR_SERVER_TYPE;
	}
	ServerTypeMap::iterator itType = m_ServerTypeMap.find(nServerType);
	if (itType != m_ServerTypeMap.end())
	{
		ServerConnMap::iterator itID = itType->second.find(nServerID);
		if (itID != itType->second.end())
		{
			CORE_DEBUG("del conn: " << nServerType << ":" << nServerID << " addrs: " << itID->second->GetInetAddr().to_string());
			itType->second.erase(itID);
		}
	}
	//调用lua的删除连接回调
	lua_tinker::call<void>(LUA()->m_L, "DelConn", nServerType, nServerID);
	return S_OK;
}

int32_t CConnectionMgr::ConnectServer(const ServerType nServerType, const uint32_t nServerID, const Inet_Addr& addr)
{
	//已经有到这个server的连接
	CConnection* conn = GetConn(nServerType, nServerID);
	if (conn != NULL)
	{
		return S_OK;
	}

	conn = m_ConnPool.pop_obj();
	if (conn == NULL)
	{
		CORE_ERROR("addr" << addr.to_string() << "get conn error!");
		return ERROR_GET_CONN;
	}
	int32_t ret = conn->Connect(NET_THREAD_INSTANCE()->GetEvBase(), addr);
	if (ret < 0)
	{
		CORE_ERROR("connect " << addr.to_string() << "failed!! : " << ret);
		conn->Init();
		m_ConnPool.push_obj(conn);
		return ERROR_CONNECT_CONN;
	}
	conn->m_ServerType = nServerType;
	conn->m_ServerID = nServerID;
	AddConn(nServerType, nServerID, conn);
	conn->SendPing();
	return S_OK;
}

int32_t CConnectionMgr::SendToServer(const ServerType nServerType, const uint32_t nServerID, const void *data, size_t size)
{
	CConnection* conn = GetConn(nServerType, nServerID);
	if (conn == NULL)
	{
		CORE_ERROR("get server : " << nServerType <<" : " << nServerID << "conn error !");
		return ERROR_GET_CONN;
	}
	return conn->Send(data, size);
}

int32_t CConnectionMgr::SendToServer(const ServerType nServerType, const uint32_t nServerID, CBasePacket& basePack)
{
	CConnection* conn = GetConn(nServerType, nServerID);
	if (conn == NULL)
	{
		CORE_ERROR("get server : " << (uint32_t)nServerType << " : " << nServerID << "conn error !");
		return ERROR_GET_CONN;
	}
	int32_t ret = conn->Send(basePack);

	if (ret != S_OK)
	{
		CORE_ERROR("send msg to server "<< (uint32_t)nServerType << ":" << nServerID << " error : " << ret);
	}
	return ret;
}

int32_t CConnectionMgr::SendToServer(const ServerType nServerType, CBasePacket& basePack)
{
	ServerTypeMap::iterator itType = m_ServerTypeMap.find(nServerType);
	if (itType != m_ServerTypeMap.end())
	{
		for (auto it = itType->second.begin(); it != itType->second.end();it++)
		{
			SendToServer(nServerType,it->first, basePack);
		}
	}
	return S_OK;
}

 CConnection* CConnectionMgr::GetConn(const ServerType nServerType, const uint32_t nServerID)
{
	ServerTypeMap::iterator itType = m_ServerTypeMap.find(nServerType);
	if (itType != m_ServerTypeMap.end())
	{
		ServerConnMap::iterator itID = itType->second.find(nServerID);
		if (itID != itType->second.end())
		{
			return itID->second;
		}
	}
	return NULL;
}
 uint32_t CConnectionMgr::GetConnCount(const ServerType nServerType)
 {
	 ServerTypeMap::iterator itType = m_ServerTypeMap.find(nServerType);
	 if (itType != m_ServerTypeMap.end())
	 {
		 return itType->second.size();
	 }
	 return 0;
 }
BASE_NAMESPACE_END_DECL

