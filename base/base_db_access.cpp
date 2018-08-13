
#include "base_db_access.h"
#include "base_date_time.h"
#include "base_lua.h"

BASE_NAMESPACE_BEGIN_DECL
// --------------------------------------------------------------------------------------
//  DBAccess
// --------------------------------------------------------------------------------------
DBAccess::DBAccess()
{
	m_dbType = SA_Oracle_Client;
	m_dbName = "";
	m_uid = "";
	m_pwd = "";
}


////////////////////////////////////////////////////////////////////////////////
/**
* 配置
* @param[in] dbName:   db名
* @param[in] username: 用户名
* @param[in] password: 密码
* @param[in] type:     数据库类型
*/
void DBAccess::Config(const char *dbName, const char *username, const char *password, SAClient_t type)
{
	if (dbName != NULL) { m_dbName = dbName; }
	if (username != NULL) { m_uid = username; }
	if (password != NULL) { m_pwd = password; }
	m_dbType = type;
	if (m_dbType == SA_MySQL_Client)
	{
#ifdef WIN32
		m_dbConn.setOption("MYSQL.LIBS") = "../../lib/libmysql.dll";
#else
		m_dbConn.setOption("MYSQL.LIBS") = "../../lib/libmysqlclient.so";
#endif
	}
}

void DBAccess::ConfigByStr(const char *dbName, const char *username, const char *password, const char *type)
{
	if (dbName != NULL) { m_dbName = dbName; }
	if (username != NULL) { m_uid = username; }
	if (password != NULL) { m_pwd = password; }

	if (type)
	{
		char t[64] = {};
		strcpy(t,  type);
		if (strcmp(t, "oracle") == 0)
		{
			m_dbType = SA_Oracle_Client;
		}
		else if (strcmp(t, "sqlserver") == 0)
		{
			m_dbType = SA_SQLServer_Client;
		}
		else if (strcmp(t, "mysql") == 0)
		{
			m_dbType = SA_MySQL_Client;
#ifdef WIN32
			m_dbConn.setOption("MYSQL.LIBS") = "../../lib/libmysql.dll";
#else
			m_dbConn.setOption("MYSQL.LIBS") = "../../lib/libmysqlclient.so";
#endif
		}
		else if (strcmp(t, "odbc") == 0)
		{
			m_dbType = SA_ODBC_Client;
		}
	}
}


/**
* 重连接db
* @return: 成功返回true
*/
bool DBAccess::Connect()
{
	SAConnection &conn = m_dbConn;

	// 关闭之前的连接
	if (conn.isConnected())
	{
		try
		{
			conn.Disconnect();
		}
		catch (SAException &)
		{
		}
	}

	try
	{
		conn.Connect(m_dbName, m_uid, m_pwd, m_dbType);
	}
	catch (SAException &e)
	{
		CORE_ERROR("connect error "<< (const char *)e.ErrText());
		return false;
	}

	return true;
}
/**
* 断开连接
*/
void DBAccess::Disconnect()
{
	SAConnection &conn = m_dbConn;
	try
	{
		conn.Disconnect();
	}
	catch (SAException &)
	{
	}
}

/**
* 执行sql
* @param[in] cmd: 执行的命令
* @return: 执行成功返回true
*/
bool DBAccess::ExecuteSQL(SACommand &cmd)
{
	SAConnection &conn = m_dbConn;

	try
	{
		if (!conn.isAlive())
		{
			Connect();
		}
		cmd.setConnection(&conn);
		cmd.Prepare();
		cmd.Execute();
		conn.Commit();
	}
	// 异常
	catch (SAException &e)
	{
		CORE_ERROR("ExecuteSQL error " << (const char *)e.ErrText());
		try
		{
			conn.Rollback();
		}
		catch (SAException &)
		{
		}
		if (Connect())
		{
			try
			{
				cmd.setConnection(&conn);
				cmd.Prepare();
				cmd.Execute();
				conn.Commit();
				return true;
			}
			catch (SAException &e)
			{
				CORE_ERROR("ExecuteSQL error " << (const char *)e.ErrText());
				try
				{
					conn.Rollback();
				}
				catch (SAException &)
				{
				}
			}
		}

		SAString sql = cmd.CommandText();
		if (sql.GetLength() > 768)
		{
			
		}
		else
		{
			
		}
		return false;
	}

	return true;
}

// 设置连接选项
void DBAccess::SetConnectionOption(const char *key, const char *value)
{
	m_dbConn.setOption(key) = value;
}

bool DBAccess::ExecuteSQL(const string& sql, string& jsStr)
{
	SACommand cmd;
	cmd.setCommandText(sql.c_str(), SA_CmdSQLStmt);
	bool bRet = ExecuteSQL(cmd);
	if (!bRet)
	{
		return bRet;
	}

	json_object* jsObject = json_object_new_array();

	if (cmd.isResultSet())
	{
		// 遍历每一条记录，n用于统计结果集的行数
		int n = 0;
		for (n = 0; cmd.FetchNext(); ++n)
		{
			json_object* subJs = json_object_new_object();
			int count = cmd.FieldCount();
			// 遍历记录的每条字段
			for (int i = 1; i <= count; ++i)
			{
				PushFieldToJson(cmd.Field(i), subJs);
			}
			json_object_array_add(jsObject, subJs);
		}
		if (n == 0)
		{
			//json_object_array_add(jsObject, json_object_new_int(n));
		}
	}
	else
	{
		
		int n = cmd.RowsAffected();
		json_object_array_add(jsObject, json_object_new_int(n));
	}

	jsStr = json_object_to_json_string(jsObject);
	json_object_put(jsObject);

	return true;
}

bool DBAccess::ExecuteProc(const string& procName, table& tableInput, table& tableOutput, string& jsStr)
{
	SACommand cmd;
	cmd.setCommandText(procName.c_str(), SA_CmdStoredProc);
	SAParam    *pParam = NULL;
	//设置参数
	bool bRet = SetCmdParam(&cmd, tableInput, tableOutput);
	if (!bRet)
	{
		return bRet;
	}

	bRet = ExecuteSQL(cmd);
	if (!bRet)
	{
		return bRet;
	}

	int count = cmd.ParamCount();
	// 遍历每个输出参数
	json_object* jsObject = json_object_new_array();
	for (int i = 0; i < count; ++i)
	{
		pParam = &cmd.ParamByIndex(i);
		if (pParam->ParamDirType() == SA_ParamInput)
		{
			continue;
		}
		if (pParam->ParamType() == SA_dtCursor)
		{
			CORE_WARNING("ExecuteProc out param can not out put SA_dtCursor!!");
			continue;
		}
		switch (pParam->ParamType())
		{
		case SA_dtLong:
			json_object_array_add(jsObject, json_object_new_int64((int)pParam->asLong()));
			break;
		case SA_dtDouble:
			json_object_array_add(jsObject, json_object_new_double(pParam->asDouble()));
			break;
		case SA_dtBool:
			json_object_array_add(jsObject, json_object_new_boolean(!!(pParam->asBool())));
			break;
		case SA_dtString:
			json_object_array_add(jsObject, json_object_new_string_len((const char *)pParam->asString(), pParam->asString().GetLength()));
			break;
		default:
			break;
		}
	}
	jsStr = json_object_to_json_string(jsObject);
	json_object_put(jsObject);
	return true;
}

bool DBAccess::ExecuteSQL(const string & sql, table & tRec)
{
	if (tRec.m_obj == NULL)
	{
		return false;
	}
	lua_State *L = tRec.m_obj->m_L;
	if (!L)
	{
		return false;
	}

	SACommand cmd;
	cmd.setCommandText(sql.c_str(), SA_CmdSQLStmt);
	bool bRet = ExecuteSQL(cmd);
	if (!bRet)
	{
		return bRet;
	}

	json_object* jsObject = json_object_new_array();

	if (cmd.isResultSet())
	{
		// 遍历每一条记录，n用于统计结果集的行数
		int n = 0;
		for (n = 0; cmd.FetchNext(); ++n)
		{
			table tSubRec(LUA()->m_L);
			int count = cmd.FieldCount();
			// 遍历记录的每条字段
			for (int i = 1; i <= count; ++i)
			{
				PushFieldToTable(cmd.Field(i), tSubRec);
			}
			tRec.set(n + 1, tSubRec);
		}
		if (n == 0)
		{
			return false;
		}
	}
	else
	{
		int n = cmd.RowsAffected();
		tRec.set(1, n);
	}
	return true;
}

bool DBAccess::ExecuteProc(const string & procName, table & tableInput, table & tableOutput, table & tRec)
{
	SACommand cmd;
	cmd.setCommandText(procName.c_str(), SA_CmdStoredProc);
	SAParam    *pParam = NULL;
	//设置参数
	bool bRet = SetCmdParam(&cmd, tableInput, tableOutput);
	if (!bRet)
	{
		return bRet;
	}

	bRet = ExecuteSQL(cmd);
	if (!bRet)
	{
		return bRet;
	}
	int32_t	iRec = 0;
	int count = cmd.ParamCount();
	// 遍历每个输出参数
	for (int i = 0; i < count; ++i)
	{
		pParam = &cmd.ParamByIndex(i);
		if (pParam->ParamDirType() == SA_ParamInput)
		{
			continue;
		}
		if (pParam->ParamType() == SA_dtCursor)
		{
			CORE_WARNING("ExecuteProc out param can not out put SA_dtCursor!!");
			continue;
		}
		switch (pParam->ParamType())
		{
		case SA_dtLong:
			bRet = true;
			iRec += 1;
			tRec.set(iRec, (int64_t)pParam->asLong());
			break;
		case SA_dtDouble:
			bRet = true;
			iRec += 1;
			tRec.set(iRec, pParam->asDouble());
			break;
		case SA_dtBool:
			bRet = true;
			iRec += 1;
			tRec.set(iRec, !!(pParam->asBool()));
			break;
		case SA_dtString:
			bRet = true;
			iRec += 1;
			tRec.set(iRec, (const char *)pParam->asString(), pParam->asString().GetLength());
			break;
		default:
			break;
		}
	}
	return true;
}

void DBAccess::PushTableToParam(SACommand* cmd,table& valueTable, const string& key, const int keyType, const int valueType, const eSAParamDirType paramType, bool setSize)
{
	if (cmd == NULL)
	{
		return;
	}
	static uint32_t keyCount = 0;
	if (keyCount >= 0x0fffffff)
	{
		keyCount = 0;
	}
	SAParam* pParam = NULL;
	char paramKeyBuf[32] = {};
	sprintf(paramKeyBuf, "%d", keyCount++);
	
	switch (valueType)
	{
	case CPP_INT:
	{
		pParam = &cmd->CreateParam(paramKeyBuf, SA_dtLong, paramType);
		pParam->setAsLong() = GetValue(valueTable, key.c_str(), keyType, int);
		break;
	}
	case CPP_FLOAT:
	{
		pParam = &cmd->CreateParam(paramKeyBuf, SA_dtDouble, paramType);
		pParam->setAsDouble() = GetValue(valueTable, key.c_str(), keyType, float);
		break;
	}
	case CPP_BOOL:
	{
		pParam = &cmd->CreateParam(paramKeyBuf, SA_dtBool, paramType);
		pParam->setAsBool() = GetValue(valueTable, key.c_str(), keyType, bool);
		break;
	}
	case CPP_STRING:
	{
		pParam = &cmd->CreateParam(paramKeyBuf, SA_dtString, paramType);
		pParam->setAsString() = GetValue(valueTable, key.c_str(), keyType, char*);
		break;
	}

	default:
		break;
	}

	if (setSize && pParam != NULL)
	{
		pParam->setParamSize(valueTable.get<int>("size"));
	}
}

bool DBAccess::SetCmdParam(SACommand * cmd, table & tableInput, table & tableOutput)
{
	if (cmd == NULL)
	{
		return false;
	}
	//添加传入参数
	TABLE_FOREACH_BEGIN(key, keyType, valueType, tableInput);
	{
		PushTableToParam(cmd, tableInput, key, keyType, valueType, SA_ParamInput);
	}
	TABLE_FOREACH_END
		//添加用于存储返回值的param
		TABLE_FOREACH_BEGIN(key, keyType, valueType, tableOutput);
	{
		//表示需要设置存储输出的大小（通常用于字符串）
		if (valueType == CPP_TABLE)
		{
			table tableSet = GetValue(tableOutput, key.c_str(), keyType, table);
			PushTableToParam(cmd, tableSet, "value", CPP_STRING, CPP_STRING, SA_ParamOutput, true);
		}
		else
		{
			PushTableToParam(cmd, tableInput, key, keyType, valueType, SA_ParamOutput);
		}
	}
	TABLE_FOREACH_END
	return true;
}

bool DBAccess::PushFieldToJson(const SAField& pf, json_object* jsObject)
{
	if (jsObject == NULL)
	{
		return false;
	}
	SADataType_t type = pf.FieldType();
	if (pf.isNull())
	{
		return false;
	}
	
	switch (type)
	{
	case SA_dtShort:
	case SA_dtUShort:
	case SA_dtLong:
	case SA_dtULong:
		json_object_object_add(jsObject, pf.Name(), json_object_new_int64((int64_t)pf.asLong()));
		break;
	case SA_dtDouble:
		json_object_object_add(jsObject, pf.Name(), json_object_new_double(pf.asDouble()));
		break;
	case SA_dtNumeric:
		json_object_object_add(jsObject, pf.Name(), json_object_new_double((double)(pf.asNumeric())));
		break;
	case SA_dtBool:
		json_object_object_add(jsObject, pf.Name(), json_object_new_boolean(!!(pf.asBool())));
		break;
	case SA_dtDateTime:
	{
		DateTime date = DateTime(pf.asDateTime().GetYear(),pf.asDateTime().GetMonth(),pf.asDateTime().GetDay(),pf.asDateTime().GetHour(),pf.asDateTime().GetMinute(),pf.asDateTime().GetSecond());
		string dataStr = date.ToString();
		json_object_object_add(jsObject, pf.Name(), json_object_new_string_len(dataStr.c_str(), dataStr.length()));
		break;
	}
	case SA_dtString:
		json_object_object_add(jsObject, pf.Name(), json_object_new_string_len((const char *)pf.asString(), pf.asString().GetLength()));
		break;
	case SA_dtCLob:
		json_object_object_add(jsObject, pf.Name(), json_object_new_string_len((const char *)pf.asCLob(), pf.asCLob().GetLength()));
		break;
	case SA_dtLongChar:
		json_object_object_add(jsObject, pf.Name(), json_object_new_string_len((const char *)pf.asLongChar(), pf.asLongChar().GetLength()));
		break;
	case SA_dtBLob:
		json_object_object_add(jsObject, pf.Name(), json_object_new_string_len((const char *)pf.asBLob(), pf.asBLob().GetLength()));
		break;
	case SA_dtLongBinary:
		json_object_object_add(jsObject, pf.Name(), json_object_new_string_len((const char *)pf.asLongBinary(), pf.asLongBinary().GetLength()));
		break;
	default:
		return false;
	}

	return true;
}
bool DBAccess::PushFieldToTable(const SAField & pf, table & tSubRec)
{
	if (tSubRec.m_obj == NULL)
	{
		return false;
	}
	lua_State *L = tSubRec.m_obj->m_L;
	if (!L )
	{
		return false;
	}
	SADataType_t type = pf.FieldType();
	if (pf.isNull())
	{
		return false;
	}
	switch (type)
	{
	case SA_dtShort:
	case SA_dtUShort:
	case SA_dtLong:
	case SA_dtULong:
		tSubRec.set(pf.Name(), (int64_t)pf.asLong());
		break;
	case SA_dtDouble:
		tSubRec.set(pf.Name(), pf.asDouble());
		break;
	case SA_dtNumeric:
		tSubRec.set(pf.Name(), (double)pf.asNumeric());
		break;
	case SA_dtBool:
		tSubRec.set(pf.Name(), !!pf.asBool());
		break;
	case SA_dtDateTime:
	{
		DateTime date = DateTime(pf.asDateTime().GetYear(), pf.asDateTime().GetMonth(), pf.asDateTime().GetDay(), pf.asDateTime().GetHour(), pf.asDateTime().GetMinute(), pf.asDateTime().GetSecond());
		string dataStr = date.ToString();
		tSubRec.set(pf.Name(), dataStr.c_str(), dataStr.length());
		break;
	}
	case SA_dtString:
		tSubRec.set(pf.Name(), (const char *)pf.asString(), pf.asString().GetLength());
		break;
	case SA_dtCLob:
		tSubRec.set(pf.Name(), (const char *)pf.asCLob(), pf.asCLob().GetLength());
		break;
	case SA_dtLongChar:
		tSubRec.set(pf.Name(), (const char *)pf.asLongChar(), pf.asLongChar().GetLength());
		break;
	case SA_dtBLob:
		tSubRec.set(pf.Name(), (const char *)pf.asBLob(), pf.asBLob().GetLength());
		break;
	case SA_dtLongBinary:
		tSubRec.set(pf.Name(), (const char *)pf.asLongBinary(), pf.asLongBinary().GetLength());
		break;
	default:
		return false;
	}
	return false;
}
BASE_NAMESPACE_END_DECL
