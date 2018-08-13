/*
 * core_frame.cpp
 *
 *  Created on: 2016-3-9
 *      Author: luochj
 */

#include "core_frame.h"
#include "base_server_info.h"
#include "base_server_info.h"
#include <stdio.h>
#include <string>

using namespace std;
BASE_NAMESPACE_BEGIN_DECL

#ifdef WIN32
void ignore_pipe()
{
}

int32_t core_main()
{
	SetConsoleTitle(SERVER_INFO()->GetServerInfoStr());
	char str[15];
	bool exit = false;
	while(!exit)
	{
		fgets(str,15, stdin);
		string strIn = str;
		if (strIn == "reload\n")
		{
			//创建定时器rolad，保证一个线程处理
			TIMER_MGR()->CreateTimer(1000, LUA_RELOAD_HANDER());
		}
		else if(strIn == "quit\n")
		{
			exit = true;
		}
		usleep(100000);
	}

	return 0;
}
#else
#define SIG_PROG_EXIT	41

static bool TERMINATE_OBJ = false;

static void sig_exit_proc(int signo)
{
	if (signo == SIG_PROG_EXIT)
	{
		TERMINATE_OBJ = true;
	}

	cout << "recive signo = " <<  signo << "TERMINATE_OBJ" << TERMINATE_OBJ<< endl;
}

#define SIG_PROG_RELOAD	43

static void sig_reload_proc(int signo)
{
	if (signo == SIG_PROG_RELOAD)
	{
		TIMER_MGR()->CreateTimer(1000, LUA_RELOAD_HANDER());
	}
}

void handle_pipe(int sig)
{
	//cout << "handler pipe OK!!" << endl;
}

void ignore_pipe() //忽略PIPE信号
{
	struct sigaction action;
	action.sa_handler = handle_pipe;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	sigaction(SIGPIPE, &action, NULL);
}

int core_main()
{
	if (signal(SIG_PROG_EXIT, sig_exit_proc) == SIG_ERR)
	{
		cout << "signal failed!!!" << endl;
		return 1;
	}
	if (signal(SIG_PROG_RELOAD, sig_reload_proc) == SIG_ERR)
	{
		cout << "signal failed!!!" << endl;
		return 1;
	}

	while(!TERMINATE_OBJ)
	{
		usleep(10000);
	}

	cout << "core exit" << endl;
	return 0;
}

#endif

void CCoreFrame::Init(uint16_t port)
{
	init_socket();
	ignore_pipe();
	LOG_THREAD_CREATE();
	NET_THREAD_CREATE();
	TIMER_MGR_CREATE()->Init(NET_THREAD_CREATE()->GetEvBase());
	CREATE_LUA();
	OnInit();
	if(port >0 )
	{
		NET_THREAD_INSTANCE()->OpenListener(port);
	}
}

void CCoreFrame::Destroy()
{
	OnDestroy();
	TIMER_MGR_DESTROY();
	NET_THREAD_INSTANCE()->terminate();
	LOG_THREAD_INSTANCE()->terminate();
	DESTROY_LUA();
	NET_THREAD_DESTROY();
	LOG_THREAD_DESTROY();
	destroy_socket();
}

void CCoreFrame::Start(bool server)
{
	LOG_THREAD_INSTANCE()->start();
	NET_THREAD_INSTANCE()->start();
	OnStart();
	CORE_INFO("server : " << (uint32_t)SERVER_INFO()->nServerType << ":"<< SERVER_INFO()->nServerID << " start!");
	if(server)
	{
		core_main();
	}
}

void CCoreFrame::Stop()
{
	CORE_DEBUG("server  stop!! " );
	Destroy();
}

int32_t CCoreFrame::SendMsg(const uint32_t nServerType, const uint32_t nServerID, const uint32_t nSessionID,  const MsgType nMsgType, const uint32_t nMsgID, const char * data,int32_t nLength)
{
	if (data == NULL)
	{
		CORE_DEBUG( "send msg data is NULL" << nServerType << ":" << nServerID << ":" << nMsgID);
		return ERROR_DATA_NULL;
	}

	INIT_BASE_PACK(pack, nMsgType, nMsgID, nSessionID);
	pack.data_.assign(data, nLength);
	if (nServerID == 0)
	{
		return CONNECT_MGR()->SendToServer(nServerType, pack);
	}

	return CONNECT_MGR()->SendToServer(nServerType, nServerID, pack);
}

BASE_NAMESPACE_END_DECL
