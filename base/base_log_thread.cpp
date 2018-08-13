#include "base_log_thread.h"
#include "base_log.h"

ObjectMutexPool<LogInfoData, BaseThreadMutex, LOG_POOL_SIZE>	LOGPOOL;

BaseLogThread::BaseLogThread()
{
	set_thread_name("./log_thread");
}

BaseLogThread::~BaseLogThread()
{
	clear();
}

void BaseLogThread::clear()
{
	LogInfoData* data = NULL;
	while(queue_.get(data))
	{
		data->reset();
		LOGPOOL.push_obj(data);
	}
}

void BaseLogThread::put_log(LogInfoData* data)
{
	if(!queue_.put(data))
	{
		data->reset();
		LOGPOOL.push_obj(data);
	}
}

//用于非正常退出！！！
void BaseLogThread::stop()
{
	terminated_ = true;
}


void BaseLogThread::execute()
{
	LogInfoData* data = NULL;
	bool needEnter = true;

	while(!get_terminated() || needEnter)
	{
		data = NULL;

		if(get_terminated())
		{
			needEnter = false;
		}

		if(queue_.get(data))
		{
			if(data == NULL)
			{
				continue;
			}

			BaseLog* log = LOG_INSTANCE()->get_log_handler(data->index);
			if(log != NULL)
			{
				log->write_log(data->str_log);
			}

			data->reset();
			LOGPOOL.push_obj(data);
		}
		else
		{
			usleep(1000);
		}
	}
}

