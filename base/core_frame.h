/*
 * core_frame.h
 *
 *  Created on: 2016-3-8
 *      Author: luochj
 */

#ifndef CORE_FRAME_H_
#define CORE_FRAME_H_

#include "base_namespace.h"
#include "base_log_thread.h"
#include "base_net_thread.h"
#include "base_singleton.h"
#include "base_os.h"
#include "base_lua.h"
#include "base_timer_mgr.h"
#include "base_connection_mgr.h"

BASE_NAMESPACE_BEGIN_DECL

void ignore_pipe();
int32_t core_main();

class CCoreFrame
{
public:
	CCoreFrame()
	{	
	}
	virtual ~CCoreFrame()
	{
		Stop();
	}
	// 初始化函数 port 为监听的端口
	void 	Init(uint16_t port = 0);
	void 	Destroy();
	void Start(bool server = true);
	void Stop();
	virtual void WaitPid(){};

	//发送消息
	int32_t SendMsg(const uint32_t nServerType, const uint32_t nServerID, const uint32_t nSessionID, const MsgType nMsgType, const uint32_t nMsgID, const char* data, int32_t nLength);

protected:
	//上层需要实现的函数，用于初始化
	virtual void OnInit() = 0;
	virtual void OnStart() = 0;
	virtual void OnDestroy() = 0;
};

BASE_NAMESPACE_END_DECL

#endif /* CORE_FRAME_H_ */
