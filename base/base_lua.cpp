/*
 * core_lua.cpp
 *
 *  Created on: 2015-12-15
 *      Author: luochj
 */

#include "base_lua.h"
#include "base_def.h"
#include "json_lua.h"
#include "base_log_macro.h"
#include "base_connection_mgr.h"
#include "system_cookie.h"
#include "uuid.h"

BASE_NAMESPACE_BEGIN_DECL

CBaseLua::CBaseLua()
{
	m_luaDirs.clear();
	m_L = NULL;
	m_L = luaL_newstate();

	luaopen_base(m_L);
	luaL_openlibs(m_L);  // 保证可以使用库
	luaopen_string(m_L);
}
CBaseLua::~CBaseLua()
{
	lua_close(m_L);
}

int32_t CBaseLua::Init(vector<string>& luaDirs)
{
	int32_t ret = S_OK;
	string exePath;
	get_fullexepath(exePath);

	for (vector<string>::iterator it = luaDirs.begin(); it != luaDirs.end(); it++)
	{
		m_luaDirs.push_back(exePath + *it);
	}
	ret = ReLoad();
	DefBaseFunc();
	return ret;
}
int32_t CBaseLua::LoadFile(const string& dir)
{
	if (m_L == NULL)
	{
		CORE_ERROR("load lua error , m_L is NULL!");
		return ERROR_LOAD_LUA;
	}
	vector<string> file_names;
	//预留出data存放数据
	get_files(dir, file_names, "lua", "data");
	//加载lua文件
	for(vector<string>::iterator it = file_names.begin(); it != file_names.end(); it++)
	{
		dofile(m_L, (*it).c_str());
		CORE_INFO("load lua:" << *it);
	}
	return S_OK;
}
void CBaseLua::Close()
{
	lua_close(m_L);
}
//重新加载lua文件
int32_t CBaseLua::ReLoad()
{
	//luatinker的锁，防止正在调用的时候Reload
	//BaseGuard<BaseThreadMutex> mutex(*s_lua_mutex);

	for (vector<string>::iterator it = m_luaDirs.begin(); it != m_luaDirs.end(); it++)
	{
		vector<string> file_dirs;
		get_file_dir(*it, file_dirs, "data");
		for (vector<string>::iterator it = file_dirs.begin(); it != file_dirs.end(); it++)
		{
			SetLuaPath(*it);
		}
	}

	for (vector<string>::iterator it = m_luaDirs.begin(); it != m_luaDirs.end(); it++)
	{
		LoadFile(*it);
	}
	cout << "load lua ok!!!" << endl;
	return S_OK;
}

int CBaseLua::SetLuaPath(const string& path )
{
	lua_getglobal( m_L, "package" );
	lua_getfield( m_L, -1, "path" );
	string cur_path = lua_tostring( m_L, -1 );
	cur_path.append( ";" );
	cur_path.append( path );
	cur_path.append( "?.lua" );
	lua_pop( m_L, 1 );
	lua_pushstring( m_L, cur_path.c_str() );
	lua_setfield( m_L, -2, "path" );
	lua_pop( m_L, 1 );
	return S_OK;
}

void CBaseLua::DefBaseFunc()
{
	//添加数据库类到lua
	class_add<DBAccess>(m_L, "DBAccess");
	class_con<DBAccess>(m_L, constructor<DBAccess>);
	class_def<DBAccess>(m_L, "SetConnectionOption", &DBAccess::SetConnectionOption);
	class_def<DBAccess>(m_L, "Config", &DBAccess::ConfigByStr);
	class_def<DBAccess>(m_L, "Connect", &DBAccess::Connect);
	class_def<DBAccess>(m_L, "Disconnect", &DBAccess::Disconnect);

	class_add<CConnection>(m_L, "CConnection");
	class_con<CConnection>(m_L, constructor<CConnection>);
	class_def<CConnection>(m_L, "SendMsg", &CConnection::LSend);
	class_def<CConnection>(m_L, "SendByMsgID", &CConnection::LSendByMsgID);
	class_def<CConnection>(m_L, "GetRemoteAddr", &CConnection::LGetRemoteAddr);
	class_def<CConnection>(m_L, "SetTypeID", &CConnection::LSetTypeID);
	class_def<CConnection>(m_L, "GetType", &CConnection::GetType);
	class_def<CConnection>(m_L, "GetID", &CConnection::GetID);

	class_add<CNodeLoadManager>(m_L, "CNodeLoadManager");
	class_con<CNodeLoadManager>(m_L, constructor<CNodeLoadManager>);
	class_def<CNodeLoadManager>(m_L, "UpdateNode", &CNodeLoadManager::LUpdateNode);
	class_def<CNodeLoadManager>(m_L, "SelectNode", &CNodeLoadManager::LSelectNode);
	class_def<CNodeLoadManager>(m_L, "DelNode", &CNodeLoadManager::del_node);
	


	class_add<CBaseLua>(m_L, "Clua");
	class_def<CBaseLua>(m_L, "JsonToTable", &CBaseLua::LJsonToTable);
	class_def<CBaseLua>(m_L, "CreateTimer", &CBaseLua::LCreateTimer);
	class_def<CBaseLua>(m_L, "DelTimer", &CBaseLua::LDelTimer);
	class_def<CBaseLua>(m_L, "TableTest", &CBaseLua::LTableTest);
	class_def<CBaseLua>(m_L, "ExecuteSql", &CBaseLua::LExecuteSql);
	class_def<CBaseLua>(m_L, "ExecuteProc", &CBaseLua::LExecuteProc);
	class_def<CBaseLua>(m_L, "WriteLog", &CBaseLua::LWriteLog);
	class_def<CBaseLua>(m_L, "ConnectServer", &CBaseLua::LConnectServer);
	class_def<CBaseLua>(m_L, "GetConnIP", &CBaseLua::LGetConnIP);
	class_def<CBaseLua>(m_L, "GetNewID", &CBaseLua::LGetNewID);
	class_def<CBaseLua>(m_L, "Randi", &CBaseLua::LRandi);
	class_def<CBaseLua>(m_L, "CreateCookie", &CBaseLua::LCreateCookie);
	class_def<CBaseLua>(m_L, "CheckCookie", &CBaseLua::LCheckCookie);
	class_def<CBaseLua>(m_L, "SetLogLevel", &CBaseLua::LSetLogLevel);
	class_def<CBaseLua>(m_L, "Sleep", &CBaseLua::LSleep);
	class_def<CBaseLua>(m_L, "GetUUID", &CBaseLua::LGetUUID);
	class_def<CBaseLua>(m_L, "CreateLodeMgr", &CBaseLua::LCreateLodeMgr);
	class_def<CBaseLua>(m_L, "GetConnCount", &CBaseLua::LGetConnCount);
	class_def<CBaseLua>(m_L, "GetServerFlow", &CBaseLua::LGetServerFlow);
	class_def<CBaseLua>(m_L, "GetNowMilliSecond", &CBaseLua::LGetNowMilliSecond);
	
	
	lua_tinker::set(m_L, "clua", LUA());

}

table CBaseLua::LJsonToTable(const char *jchar)
{
	table retTable(m_L);
	if (CJsonLuaTable::JsonToTable(jchar, retTable))
	{
		return retTable;
	}
	return nillTable(m_L);
}

uint32_t CBaseLua::LCreateTimer(const float nTime, const bool bOnce)
{
	return TIMER_MGR()->CreateTimer(nTime, LUA_TIMER_HANDER(), NULL, bOnce);
}

void CBaseLua::LDelTimer(const uint32_t nTimerID)
{
	TIMER_MGR()->DelTimer(nTimerID);
}

table CBaseLua::LExecuteSql(DBAccess& dbConnect, const char* sql)
{
	if (sql == NULL)
	{
		LOG_ERROR("sql is NULL");
		return nillTable(m_L);
	}
	table retTable(m_L);
	if (dbConnect.ExecuteSQL(sql, retTable))
	{
		return retTable;
	}
	return nillTable(m_L);
}
bool CBaseLua::LExecuteProc(DBAccess& dbConnect, const char *sql, table tableOuput, table tableInput)
{
	if (sql == NULL)
	{
		LOG_ERROR("sql is NULL");
		return false;
	}
	return dbConnect.ExecuteProc(sql, tableInput, tableOuput, tableOuput);
	//此处省略3000字，解释为什么用这个做返回，简而言之，尼玛不知道为什么用其他的table lua收不到
}

table CBaseLua::LTableTest(table table1, table table2)
{
	DBAccess dbAccess;
	dbAccess.SetConnectionOption("HandleResult", "store");
	dbAccess.SetConnectionOption("CharacterSet", "utf8");
	dbAccess.Config("192.168.1.100,33306@dev_hog_gamedb_119", "root", "pixss", SA_MySQL_Client);
	dbAccess.Connect();

	string jsStr = "";
	dbAccess.ExecuteProc("p_get_ex_count", table1, table2,jsStr);
	CORE_DEBUG("select result: " << jsStr);
	return table2;
}

void CBaseLua::LWriteLog(const int32_t logLevel, const char* funName, const char* log)
{
	switch (logLevel)
	{
	case BaseLogStreamInterface::debug:
	{
		LOG_DEBUG(funName<<": "<< log);
		break;
	}
	case BaseLogStreamInterface::general:
	{
		LOG_INFO(funName << ": " << log);
		break;
	}
	case BaseLogStreamInterface::warning:
	{
		LOG_WARNING(funName << ": " << log);
		break;
	}
	case BaseLogStreamInterface::error:
	{
		LOG_ERROR(funName << ": " << log);
		break;
	}
	case BaseLogStreamInterface::fatal:
	{
		LOG_FATAL(funName << ": " << log);
		break;
	}
	default:
		break;
	}
}
int32_t CBaseLua::LConnectServer(const uint32_t nServerType, const uint32_t nServerID, const char * ip, uint16_t port)
{
	if (ip == NULL)
	{
		LOG_ERROR("connect " << nServerType << ":" << nServerID << " ip is null" );
		return ERROR_CONNECT_CONN;
	}
	string hostname = ip;
	Inet_Addr addr(hostname,port);

	return CONNECT_MGR()->ConnectServer(nServerType, nServerID, addr);
}
table CBaseLua::LGetConnIP(const uint32_t nServerType, const uint32_t nServerID)
{
	CConnection* conn = CONNECT_MGR()->GetConn(nServerType, nServerID);
	if (conn == NULL)
	{
		return nillTable(m_L);
	}
	string ip = conn->GetInetAddr().ip_to_string();
	table ret(m_L);
	ret.set("ip", ip.c_str());
	return ret;
}
uint32_t CBaseLua::LGetNewID()
{
	static uint32_t id = 0;
	return InterlockedIncrement(&id);
}

int32_t CBaseLua::LRandi(int32_t minVal, int32_t maxVal)
{
	int32_t v1 = MIN(minVal, maxVal);
	int32_t v2 = MAX(minVal, maxVal);
	uint32_t r = 0 ;
	rand_s(&r);
	return (int32_t)(r % (v2 - v1 + 1)) + v1;
}
table CBaseLua::LCreateCookie(RoleID nRoleID)
{
	string cookie = create_login_cookie(nRoleID);
	table ret(m_L);
	ret.set("cookie", cookie.c_str(),cookie.size());
	return ret;
}
int32_t CBaseLua::LCheckCookie(RoleID nRoleID, const char * pCookie)
{
	return check_login_cookie(get_encrypt_version(), pCookie, nRoleID);
}
void CBaseLua::LSetLogLevel(uint32_t nLevel)
{
	SET_CORE_LOG_LEVEL(nLevel);
	SET_SERVER_LOG_LEVEL(nLevel);
}
void CBaseLua::LSleep(uint32_t ms)
{
	usleep(ms*1000);
}
uint64_t CBaseLua::LGetUUID()
{
	uint64_t nSpecialNum = SERVER_INFO()->nServerType;
	nSpecialNum = nSpecialNum << 7;
	nSpecialNum |= SERVER_INFO()->nServerID;
	return UUID()->GetNewID(nSpecialNum);
}
CNodeLoadManager * CBaseLua::LCreateLodeMgr()
{
	return m_LodeMgrPool.pop_obj();
}

uint32_t CBaseLua::LGetConnCount(const ServerType nServerType)
{
	return CONNECT_MGR()->GetConnCount(nServerType);
}

table CBaseLua::LGetServerFlow()
{
	table ret(m_L);
	ret.set("inf", SERVER_INFO()->nRecvBitCount);
	ret.set("out", SERVER_INFO()->nSendBitCount);
	SERVER_INFO()->nRecvBitCount = 0;
	SERVER_INFO()->nSendBitCount = 0;
	return ret;
}

uint64_t CBaseLua::LGetNowMilliSecond()
{
	uint64_t ret = DateTime::GetMillisecond();
	return ret;
}

BASE_NAMESPACE_END_DECL


