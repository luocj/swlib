/*
 * core_frame.h
 *
 *  Created on: 2016-3-8
 *      Author: luochj
 */

#ifndef BASE_SERVER_INFO_H_
#define BASE_SERVER_INFO_H_

#include "base_namespace.h"
#include "base_singleton.h"
#include "base_os.h"
#include "base_typedef.h"
#include "base_def.h"
#include "base_lua.h"

BASE_NAMESPACE_BEGIN_DECL

static char* ServerName[] =
{
	"客户端",
	"测试服务器",
	"精灵服务器",
	"网关服务器",
	"登录服务器",
	"游戏服务器",
	"战斗服务器",
	"数据库服务器"
};

class CServerInfo
{
public:
	CServerInfo()
	{
		nServerType = enmServerType_Nothing;
		nServerID = 0;
		nRecvBitCount = 0;
		nSendBitCount = 0;
	}
public:
	char* GetServerInfoStr()
	{
		static char pStr[40] = {};
		char * pAreaName = get<char*>(LUA()->m_L, "AREA_NAME");
		if (pAreaName == NULL)
		{
			pAreaName = "";
		}
		char * pGameName = get<char*>(LUA()->m_L, "GAME_NAME");
		if (pGameName == NULL)
		{
			pGameName = "";
		}
		sprintf(pStr, "%s : %s[%s] : %d", pGameName, ServerName[nServerType], pAreaName, nServerID);
		return pStr;
	}
public:
	ServerType  nServerType;			//服务器类型
	uint32_t	nServerID;				//服务器ID
	uint16_t	nPort;					//端口
	uint64_t	nRecvBitCount;			//收到的数据大小
	uint64_t	nSendBitCount;			//发送的数据大小
};

#define SERVER_INFO_CREATE			CSingleton<CServerInfo>::instance
#define SERVER_INFO					CSingleton<CServerInfo>::instance
#define SERVER_INFO_DESTROY			CSingleton<CServerInfo>::destroy

BASE_NAMESPACE_END_DECL

#endif /* BASE_SERVER_INFO_H_ */
