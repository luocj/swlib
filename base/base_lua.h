/*
 * core_lua.h
 *	lua的一些公共接口定义
 *  Created on: 2015-12-15
 *      Author: luochj
 */


#ifndef BASE_LUA_H_
#define BASE_LUA_H_

#define _CRT_RAND_S 
#include <stdlib.h>

#include<vector>
#include<string>
#include<string>
#include<iostream>

#include "base_namespace.h"
#include "base_typedef.h"
#include "base_singleton.h"
#include "base_os.h"
#include "base_log_macro.h"
#include "base_timer_mgr.h"
#include "base_db_access.h"
#include "base_nodes_load.h"

#include "../vendor/lua/lua_tinker.h"

using namespace std;
using namespace lua_tinker;
BASE_NAMESPACE_BEGIN_DECL

class CBaseLua
{
public:
	CBaseLua();
	~CBaseLua();
	/*初始化lua
	*
	* luaDir 上层逻辑lua地址
	* gluaDir lua公共定义的地址
	*/

	int32_t	Init(vector<string>& luaDirs);
	void	Close();

	table	LJsonToTable(const char *jchar);

	uint32_t LCreateTimer(const float nTime,const bool bOnce);

	void	LDelTimer(const uint32_t nTimerID);

	table	LExecuteSql(DBAccess& dbConnect,	const char* sql);
	bool	LExecuteProc(DBAccess& dbConnect, const char* sql, table tableOuput , table tableInput);

	table	LTableTest(table table1, table table2);

	//写日志
	void	LWriteLog(const int32_t logLevel ,const char* funName, const char* log);

	//连接服务器
	int32_t LConnectServer(const uint32_t nServerType, const uint32_t nServerID, const char* ip ,uint16_t port);

	table LGetConnIP(const uint32_t nServerType, const uint32_t nServerID);

	uint32_t LGetNewID();

	int32_t LRandi(int32_t minVal, int32_t maxVal);
	//创建玩家的cookie
	table LCreateCookie(RoleID nRoleID);
	//验证玩家的cookie
	int32_t LCheckCookie(RoleID nRoleID,const char* pCookie);
	//设置日志等级
	void LSetLogLevel(uint32_t nLevel);
	//sleep
	void LSleep(uint32_t ms);
	//生成不重复的UUID
	uint64_t LGetUUID();
	//创建节点负载管理器
	CNodeLoadManager* LCreateLodeMgr();
	//获得某种类型的连接数量
	uint32_t LGetConnCount(const ServerType nServerType);
	//获取服务器发送接收数据的大小并回0
	table LGetServerFlow();
	uint64_t LGetNowMilliSecond();
	//重新加载lua文件
	int32_t ReLoad();
private:
	//加载lua文件
	int32_t	LoadFile(const string& dir);
	//设置lua的运行查找目录
	int		SetLuaPath(const string& path );
	//注册系统级的函数到lua
	void	DefBaseFunc();
public:
	vector<string>		m_luaDirs;
//private:
	lua_State* 			m_L;
};

#define CREATE_LUA					CSingleton<CBaseLua>::instance
#define LUA							CSingleton<CBaseLua>::instance
#define DESTROY_LUA     			CSingleton<CBaseLua>::destroy


class CLuaTimerHander :public CTimerHandler
{
public:
	virtual int32_t Timeout(const void *pData, uint32_t nTimerID)
	{
		//调用lua中timer的回调函数
		call<void>(LUA()->m_L, "OnSystemTimerReach", nTimerID);
		return S_OK;
	}
};
#define LUA_TIMER_HANDER							CSingleton<CLuaTimerHander>::instance

class CLuaReloadHander :public CTimerHandler
{
public:
	virtual int32_t Timeout(const void *pData, uint32_t nTimerID)
	{
		//调用lua中timer的回调函数
		LUA()->ReLoad();
		call<void>(LUA()->m_L, "OnReload");
		return S_OK;
	}
};
#define LUA_RELOAD_HANDER							CSingleton<CLuaReloadHander>::instance

BASE_NAMESPACE_END_DECL

#endif /* CORE_LUA_H_ */
