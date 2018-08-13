/*
 * base_def.h
 *
 *  Created on: 2014-2-28
 *      Author: luocj
 */

#ifndef BASE_DEF_H_
#define BASE_DEF_H_

#define _CRT_RAND_S 
#include <stdlib.h>

#include "base_namespace.h"
#include "base_typedef.h"
#include <stdio.h>

BASE_NAMESPACE_BEGIN_DECL


#define S_OK    0
/*
 * 字符串长度相关定义
 */
enum
{
	//文件相关
	enmMaxFileNameLength					= 256,		//文件名最大长度,

	//ip相关
	enmMaxIPAddressLength				= 20,			//IP地址最大长度

	//数据库相关
	enmMaxDBUserLength					= 256,		//数据库用户名最大长度
	enmMaxDBPasswordLength			= 32,			//数据库密码最大长度
	enmMaxDBNameLength					= 64,  		 //数据库命最大长度
	enmMaxFieldValueLength				= 1024*10,	 //数据库字段值最大长度
};

typedef uint8_t						LogLevel;			//日志级别
enum
{
	enmLogLevel_Debug			= 0x00,				//调试信息
	enmLogLevel_Info				= 0x01,				//一般日志信息
	enmLogLevel_Notice			= 0x02,				//通知
	enmLogLevel_Warning		= 0x03,				//警告
	enmLogLevel_Error				= 0x04,				//错误
	enmLogLevel_Critical			= 0x05,				//严重错误
};

typedef uint8_t						ServerType;
enum
{
	enmServerType_Client			= 0x00,				//客户端
	enmServerType_TestServer		= 0x01,				//测试服务器
	enmServerType_Demon				= 0x02,				//精灵服务器
	enmServerType_Gate				= 0x03,				//网关服务器
	enmServerType_Login				= 0x04,				//登录服务器
	enmServerType_Game				= 0x05,				//游戏服务器
	enmServerType_Battle			= 0x06,				//战斗服务器
	enmServerType_DB				= 0x07,				//DBcenter

	enmServerType_Nothing			=	0xff,				//
};

typedef uint8_t						MsgType;
enum
{
	enmMsgType_Req	= 0x01,				//请求
	enmMsgType_Resp = 0x02,				//回应
	enmMsgType_Noti = 0x03,				//通知
	enmMsgType_Ping = 0x04,				//用于连接的时候设置连接信息
};

typedef uint32_t				RoleID;

//错误码
#define				ERROR_LOAD_LUA			0x80000001		//加载lua错误
#define				ERROR_SERVER_TYPE		0x80000002		//服务器类型错误
#define				ERROR_GET_CONN			0x80000003		//获取连接错误
#define				ERROR_CONNECT_CONN		0x80000004		//连接错误
#define				ERROR_DATA_NULL			0x80000005		//发送的数据为null
#define				ERROR__BEV_NULL			0x80000006		//libevent的buffrr event NULL
#define				ERROR_EVENT_BASE_NULL	0x80000007		//libevent的base NULL
#define				ERROR_HTTP_NULL			0x80000008		//创建libevent的http失败

BASE_NAMESPACE_END_DECL
#endif /* BASE_DEF_H_ */
