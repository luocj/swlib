/*************************************************************************************
*filename:	base_log_macro.h
*
*to do:	框架日志
*Create on: 2016-03
*Author:	luochj
*check list:
*************************************************************************************/
#ifndef __CORE_LOG_MACRO_H
#define __CORE_LOG_MACRO_H

#include "base_log.h"

//用于框架的日志输出
extern BaseLogStream core_log;

#define CORE_DEBUG(arg)\
	DEBUG_TRACE(core_log, arg)

#define CORE_INFO(arg)\
	INFO_TRACE(core_log, arg)

#define CORE_WARNING(arg)\
	WARNING_TRACE(core_log, arg)

#define CORE_ERROR(arg)\
	ERROR_TRACE(core_log, arg)

#define CORE_FATAL(arg)\
	FATAL_TRACE(core_log, arg)

#define SET_CORE_LOG_LEVEL(arg)\
	core_log.set_trace_level(arg);


//用于上层的日志输出
extern BaseLogStream server_log;

#define LOG_DEBUG(arg)\
	DEBUG_TRACE(server_log, arg)

#define LOG_INFO(arg)\
	INFO_TRACE(server_log, arg)

#define LOG_WARNING(arg)\
	WARNING_TRACE(server_log, arg)

#define LOG_ERROR(arg)\
	ERROR_TRACE(server_log, arg)

#define LOG_FATAL(arg)\
	FATAL_TRACE(server_log, arg)

#define SET_SERVER_LOG_LEVEL(arg)\
	server_log.set_trace_level(arg);

#define  SET_LOG_NAME(arg)\
	server_log.set_log_name(arg);

#endif
/************************************************************************************/

