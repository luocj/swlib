/*
* system_cookie.h
* cookie 生成与校验
*  Created on: 2016-7-14
*      Author: luochj
*/

#ifndef __MINDUMP_H
#define __MINDUMP_H

#ifdef WIN32
#include "base_def.h"
#include "base_namespace.h"
#include "base_os.h"
#include "base_date_time.h"
#include <string>
#include <DbgHelp.h>

using namespace std;
using namespace BASE_NAMEPSACE_DECL;

// --------------------------------------------------------------------------------------
//  现场恢复
// --------------------------------------------------------------------------------------

#pragma comment(lib, "DbgHelp.lib")

class MiniDump
{
public:
	/// 构造
	/// 参数
	///   [in] path:  Dump文件的记录路径（只包含到文件的前缀名）
	MiniDump(const char *path)
	{
		memset(&_dmp_time, 0, sizeof(_dmp_time));
		memset(&_mdei, 0, sizeof(_mdei));
		memset(_dmp_path, 0, sizeof(char)*MAX_PATH);
		_dmp_result = 0;
		_path_len = 0;
		_h_dmpfile = INVALID_HANDLE_VALUE;

		if (path)
		{
			memcpy(_dmp_path, path, strlen(path));
		}
	}

public:
	/// 写入dump文件
	/// 参数
	///   [in] pExceptionInfo:  异常信息。用GetExceptionInformation()获得
	/// 返回值
	///   写入成功返回0，否则为windows错误码
	uint32_t   Write(LPEXCEPTION_POINTERS pExceptionInfo)
	{
		string final_file_name = _dmp_path;

		final_file_name += "-";
		char proStr[30] = { 0 };
		sprintf(proStr, "%d-", (int)::GetCurrentProcessId());
		final_file_name += proStr;
		final_file_name += DateTime::GetNow().ToString("%Y-%m-%d[%H-%M-%S]");
		final_file_name += ".dmp";

		_h_dmpfile = ::CreateFileA(final_file_name.c_str(), FILE_WRITE_DATA, FILE_SHARE_READ, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		//_h_dmpfile = fopen(final_file_name.c_str(), "a+");
		// 写入
		if (_h_dmpfile != INVALID_HANDLE_VALUE)
		{
			_mdei.ThreadId = ::GetCurrentThreadId();
			_mdei.ExceptionPointers = pExceptionInfo;
			_mdei.ClientPointers = FALSE;

			::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), _h_dmpfile,
				MiniDumpNormal, (pExceptionInfo != 0) ? &_mdei : 0, 0, 0);
			_dmp_result = ::GetLastError();
			::CloseHandle(_h_dmpfile);
		}
		else
		{
			_dmp_result = ::GetLastError();
		}

		_h_dmpfile = INVALID_HANDLE_VALUE;
		return _dmp_result;
	}

private:
	char							_dmp_path[MAX_PATH];        // 记录文件路径，不包括后缀和扩展名
	uint32_t                        _path_len;					// 文件路径长度
	uint32_t                        _dmp_result;
	SYSTEMTIME                      _dmp_time;
	HANDLE                          _h_dmpfile;
	MINIDUMP_EXCEPTION_INFORMATION  _mdei;
}; // end class MiniDumpA
#endif //WIN32

#endif //__MINDUMP_H

