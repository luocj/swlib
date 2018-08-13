/*
 * json_lua.h
 *
 *  Created on: 2016-3-8
 *      Author: luochj
 */

#ifndef BASE_DB_ACCESS_H_
#define BASE_DB_ACCESS_H_

#include "base_log_macro.h"
#include "../vendor/jsonc/json.h"
#include "../vendor/lua/lua_tinker.h"
#include "../sqlapi/SQLAPI.h"
#include <string>


using namespace lua_tinker;
using namespace  std;


BASE_NAMESPACE_BEGIN_DECL

// --------------------------------------------------------------------------------------
//  DBAccess
// --------------------------------------------------------------------------------------
class DBAccess
{
public:
	DBAccess();

	void Config(const char *dbName, const char *username, const char *password, SAClient_t type = SA_MySQL_Client);
	void ConfigByStr(const char *dbName, const char *username, const char *password, const char *type = "mysql");
	bool Connect();
	void Disconnect();
	bool ExecuteSQL(SACommand &cmd);
	void SetConnectionOption(const char *key, const char *value);
	//返回json结果集，方便传输协议传输，方便lua使用
	bool ExecuteSQL(const string& sql, string& jsStr);
	bool ExecuteProc(const string& procName, table& tableInput, table& tableOutput, string& jsStr);
	//放回table类型的结果集
	bool ExecuteSQL(const string& sql, table& tRec );
	bool ExecuteProc(const string& procName, table& tableInput, table& tableOutput, table& tRec);
private:
	bool SetCmdParam(SACommand* cmd, table& tableInput, table& tableOutput);
	bool PushFieldToJson(const SAField& pf, json_object* jsObject);
	bool PushFieldToTable(const SAField& pf, table& tField);
	void PushTableToParam(SACommand* cmd,table& valueTable, const string& key,const int keyType,const int valueType,const eSAParamDirType paramType,bool setSize = false);
private:
	SAClient_t      m_dbType;                           // db类型
	SAString        m_dbName;                           // db名。对oracle则是tns
	SAString        m_uid;                              // 用户名
	SAString        m_pwd;                              // 密码
	SAConnection    m_dbConn;                           // 与db的连接
};

BASE_NAMESPACE_END_DECL

#endif /* BASE_DB_ACCESS_H_ */
