#ifndef __BASE_OS_H
#define __BASE_OS_H

#include "base_typedef.h"
#include "base_def.h"
#include <time.h>
#include <string>
#include <stdio.h>
#include <vector>
#include <iostream>

using namespace std;

#define INVALID_HANDLER		-1

#undef FD_SETSIZE
#define FD_SETSIZE      512

#define MIN(a,b) ((a)>(b)?(b):(a))
#define MAX(a,b) ((a)>(b)?(a):(b))

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mswsock.h>
#include <direct.h>
#include <tchar.h>
#include <stdlib.h>

//#pragma comment(lib, "ws2_32.lib")

typedef SOCKET BASE_HANDLER;

inline int set_socket_nonblocking(BASE_HANDLER socket)
{
	int mode = 1;
	::ioctlsocket(socket, FIONBIO, (u_long FAR*)&mode);

	return 0;
}

//inline int close(BASE_HANDLER socket)
//{
//	return closesocket(socket);
//}

inline int error_no()
{
	return ::WSAGetLastError();
}

inline void usleep(uint32_t us)
{	
	if(us < 1000)
		::Sleep(1);
	else
		::Sleep(us / 1000);
}

inline int access(const char *path, int mode)
{
	return _access(path, mode);
}

/*
* Number of micro-seconds between the beginning of the Windows epoch
* (Jan. 1, 1601) and the Unix epoch (Jan. 1, 1970).
*
* This assumes all Win32 compilers have 64-bit support.
*/
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS) || defined(__WATCOMC__)
#define DELTA_EPOCH_IN_USEC 11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_USEC 11644473600000000ULL
#endif

typedef unsigned __int64 u_int64_t;

static u_int64_t filetime_to_unix_epoch(const FILETIME *ft)
{
	u_int64_t res = (u_int64_t)ft->dwHighDateTime << 32;

	res |= ft->dwLowDateTime;
	res /= 10; /* from 100 nano-sec periods to usec */
	res -= DELTA_EPOCH_IN_USEC; /* from Win epoch to Unix epoch */
	return (res);
}

inline int gettimeofday(struct timeval *tv, void *tz)
{
	FILETIME ft;
	u_int64_t tim;

	if (!tv) {
		//errno = EINVAL;
		return (-1);
	}
	::GetSystemTimeAsFileTime(&ft);
	tim = filetime_to_unix_epoch(&ft);
	tv->tv_sec = (long)(tim / 1000000L);
	tv->tv_usec = (long)(tim % 1000000L);
	return (0);
}

#define	F_OK	0
#define	X_OK	1
#define	W_OK	2
#define	R_OK	4

#define XEAGAIN			WSAEWOULDBLOCK		//无数据读写
#define XEINPROGRESS	WSAEWOULDBLOCK		//正在进行连接
#define XECONNABORTED	WSAECONNABORTED
#define XEINTR			WSAEWOULDBLOCK

#else //LINUX GCC
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>
#include<dirent.h>


typedef int		BASE_HANDLER;

#define XEAGAIN			EAGAIN		//无数据读写
#define XEINPROGRESS	EINPROGRESS		//正在进行连接
#define XECONNABORTED	ECONNABORTED
#define XEINTR			EINTR

inline int set_socket_nonblocking(BASE_HANDLER socket)
{
	int val = fcntl(socket, F_GETFL, 0);
	fcntl(socket, F_SETFL, val | O_NONBLOCK);
	
	return 0;
}

inline int error_no()
{
	return errno;
}

inline int closesocket(BASE_HANDLER socket)
{
	return close(socket);
}

#endif


inline void init_socket()
{
#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	srand((uint32_t)time(NULL));
#else
	//srand((uint32_t)time(NULL));
	//srandom((uint32_t)time(NULL));
#endif
}

inline void destroy_socket()
{

#ifdef WIN32
	WSACleanup();
#endif
}

inline const char* get_file_name(const char* pathname)
{
	if(pathname == NULL)
		return pathname;

	int32_t size = (int32_t)strlen(pathname);

	char *pos = (char *)pathname + size;

#ifdef WIN32
	while (*pos != '\\' && pos != pathname)
#else
	while (*pos != '/' && pos != pathname)
#endif
		pos--;

	if(pos == pathname)
		return pathname;
	else
		return pos + 1;
}

#define MAX_PATH 1024

//获取当前运行的程序的目录
inline const char* get_fullexepath(string& _str_path) 
{
	char buffer[MAX_PATH] = { 0 };
#ifdef WIN32
	if (GetModuleFileName(NULL, buffer, MAX_PATH))
	{
		(_tcsrchr(buffer, _T('\\')))[1] = 0;
	}
	_str_path = buffer;
#else
	char* path = getcwd(buffer, MAX_PATH);
	_str_path = path;
	_str_path += "/";
#endif

	return _str_path.c_str();
}

//在_pparent下创建文件价_pdir
inline bool create_tracedir(const char* _pdir, const char* _pparent)
{
	if (_pparent == NULL || strlen(_pparent) == 0)
		return false;

	string str_dir = _pparent;
	str_dir += _pdir;
#ifdef WIN32
	_mkdir(str_dir.c_str());
#else 
	mkdir(str_dir.c_str(), S_IRWXU);
#endif

	return true;
}

/*
* 获取文件夹下的所有文件夹
* 包含文件本身
*
*/

inline void get_file_dir(string path, vector<string>& fileDir,string ex_path = "")
{
#ifdef WIN32
	//文件句柄
	long   hFile = 0;
	//文件信息
	struct _finddata_t fileinfo;
	string pathName;
	if ((hFile = _findfirst(pathName.assign(path).append("*").c_str(), &fileinfo)) != -1)
	{
		fileDir.push_back(path);
		do
		{
			//如果是文件夹
			if ((fileinfo.attrib &  _A_SUBDIR) || strcmp(fileinfo.name, ".") == 0 && strcmp(fileinfo.name, "..") == 0)
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0 && strcmp(fileinfo.name, ".svn") != 0 && strcmp(fileinfo.name, ex_path.c_str()) != 0)
				{
					fileDir.push_back(pathName.assign(path).append(fileinfo.name).append("\\"));
				}
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
#else
	struct dirent *dirp;
	DIR *dp;
	dp = opendir(path.c_str());
	if (dp == NULL)
	{
		return;
	}
	fileDir.push_back(path);
	while ((dirp = readdir(dp)) != NULL)
	{
		string name = dirp->d_name;
		if (dirp->d_type == DT_DIR || strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
		{
			if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0 || strcmp(dirp->d_name, ".svn") == 0  || strcmp(dirp->d_name, ex_path.c_str()) == 0)
			{
				continue;
			}
			string dirPath = path + name + "/";
			fileDir.push_back(dirPath);
		}
	}
	closedir(dp);
#endif // WIN32

}

/************************************************************************/
/*  获取文件夹下所有文件名
输入：
path	:	文件夹路径
ex_path	：	排除查找的文件夹
exd		:   所要获取的文件名后缀，如jpg、png等；如果希望获取所有
文件名, exd = ""
输出：
files	:	获取的文件名列表
/************************************************************************/

inline void get_files(string path,vector<string>& files, string exd = "", string ex_path = "")
{
	string exdName;
	if (0 != strcmp(exd.c_str(), ""))
	{
		exdName = "." + exd;
	}
	else
	{
		exdName = "";
	}
#ifdef WIN32
	//文件句柄
	long   hFile = 0;
	//文件信息
	struct _finddata_t fileinfo;
	string pathName;
	if ((hFile = _findfirst(pathName.assign(path).append("*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是文件夹中仍有文件夹,迭代之
			//如果不是,加入列表
			if ((fileinfo.attrib &  _A_SUBDIR) || strcmp(fileinfo.name, ".") == 0 && strcmp(fileinfo.name, "..") == 0)
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0 && strcmp(fileinfo.name, ex_path.c_str()) != 0)
				{
					get_files(pathName.assign(path).append(fileinfo.name).append("\\"), files,exd, ex_path);
				}
			}
			else
			{
					string name = fileinfo.name;
					if (name.find(exdName.c_str()) != -1)
					{
						files.push_back(pathName.assign(path).append(fileinfo.name));
					}
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
#else
	struct dirent *dirp;
	DIR *dp;
	dp = opendir(path.c_str());
	if (dp == NULL)
	{
		return ;
	}

	while ((dirp = readdir(dp)) != NULL)
	{
		string name = dirp->d_name;
		if (dirp->d_type == DT_DIR || strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
		{
			if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0 || strcmp(dirp->d_name, ex_path.c_str()) == 0)
			{
				continue;
			}
			string newPath = path + name + "/";
			get_files(newPath, files, exd, ex_path);
		}
		else
		{
			if (name.find(exdName.c_str()) != -1)
			{
				files.push_back(path+name);
			}
		}
	}
	closedir(dp);
#endif // WIN32

}
#endif

