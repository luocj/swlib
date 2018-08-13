////////////////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "base_os.h"
#include "base_log_thread.h"
#include "base_log.h"
#include "base_date_time.h"

#define _MAX_LOGLINE    25000

const char* title_str[] = {
			"[fatal]",
			"[error]",
			"[warning]",
			"[info]",
			"[debug]"
};

const char* get_time_str(char *date_str)
{
	//date_str = DateTime::GetNow().ToString().c_str();
	return DateTime::GetNow().ToString().c_str();
}

BaseLog::BaseLog(const char *_pfile_name, bool createFile /*= true*/) :m_line_count(0) ,m_file_count(0)
{

#ifdef WIN32
	create_tracedir("log", get_fullexepath(m_file_path));
#else
	create_tracedir("log", get_fullexepath(m_file_path));
#endif

	m_filename = _pfile_name;
	m_is_change_file = false;

#ifdef WIN32
	m_file_path +=  "log\\";
#else
	m_file_path +=  "log/";
#endif

	rename_filename = "";
	day_ = DateTime::GetNow().GetDay();

	if (createFile)
	{
		init_trace_file();
	}
}

BaseLog::~BaseLog()
{
	if (m_of_file.is_open())
		m_of_file.close();
}
string BaseLog::get_log_name(bool cur_flag)
{
	string final_file_name = m_filename;
	if(!cur_flag)
	{
		final_file_name += "-";
		char date[20] = {0}; 
		sprintf(date, "%d-%d", DateTime::GetNow().GetMonth() + 1, DateTime::GetNow().GetDay());
		final_file_name += date;
	}

	final_file_name +=".log";

	return final_file_name;
}

void BaseLog::write_log(const string& str_log)
{
	if (m_is_change_file)
	{
		init_trace_file();
		m_is_change_file = false;
	}
	if (m_of_file.is_open())
	{
		m_line_count ++;
		if (m_line_count > _MAX_LOGLINE || day_ != DateTime::GetNow().GetDay() || m_is_change_file) //过了晚上12点，换文件
		{
			init_trace_file();
			day_ = DateTime::GetNow().GetDay();
		}

		char info[64] = {0};
		m_of_file << DateTime::GetNow().ToString();

		m_of_file << str_log;

		m_of_file.flush();
	}
}

void BaseLog::init_trace_file()
{	
	char newFile[1024] = {0};

	string rename_path = m_file_path + get_log_name();

	sprintf(newFile, "%s.%d", rename_path.data(), m_file_count++);
	if (m_of_file.is_open())
	{
		m_of_file.close();
	}

	string str_logfile_path = m_file_path + get_log_name(true);
	if(m_line_count > _MAX_LOGLINE || day_ != DateTime::GetNow().GetDay()) //文件满，清空文件
	{
		rename(str_logfile_path.data(), rename_filename.c_str());
		m_of_file.open(str_logfile_path.c_str(), std::ios_base::trunc|std::ios_base::out);
	}
	else //刚开始，直接追加到后面
	{
		m_of_file.open(str_logfile_path.c_str(), std::ios_base::app|std::ios_base::out);
	}

	rename_filename = newFile;

	m_line_count = 0;

}

void BaseLog::change_path(const std::string& _str_path)
{
	if (m_of_file.is_open())
		m_of_file.close();

	m_file_path = _str_path;

#ifdef WIN32
	m_file_path +=  "\\";
#else
	m_file_path +=  "/";
#endif

	init_trace_file();
}

//此函数不会完成日志文件的切换，在写下一条日志前进行文件的变更
void BaseLog::change_log_file(const string& filename)
{
	m_filename = filename;
	m_is_change_file = true;
}

////////////////////////////////////////////////////////////////////////////////////////
BaseLogManager::BaseLogManager()
{

}

BaseLogManager::~BaseLogManager()
{
	//释放文件
	for(uint32_t i = 0; i < m_log_vector.size(); ++ i)
	{
		delete m_log_vector[i];
	}
}

int32_t BaseLogManager::create_base_log(const char *pfile_name, bool createFile)
{
	int32_t index = -1;
	
	BaseLog *file_log = new BaseLog(pfile_name, createFile);
	index = m_log_vector.size();
	m_log_vector.push_back(file_log);

	return index;
}

BaseLog* BaseLogManager::get_log_handler(int32_t index)
{	
	if(index < 0 || index > (int32_t)m_log_vector.size())
	{
		return NULL;
	}

	return m_log_vector[index];
}

////////////////////////////////////////////////////////////////////////////////////////
BaseLogStream::BaseLogStream(const char* pfile_name, int32_t level, bool createFile /*= true*/)
{
	 m_level = level;
	m_log_index = LOG_INSTANCE()->create_base_log(pfile_name, createFile);
	m_strFileName = pfile_name;
}

BaseLogStream::~BaseLogStream()
{

}

ostream& BaseLogStream::dump_trace(int32_t _level)
{	
	if(_level > 0 && _level < 5)
	{
		m_strm << title_str[_level] << "\t";
	}

	return m_strm;
}

std::ostream& BaseLogStream::get_ostream()
{
	return m_strm;
}

void BaseLogStream::put_log(int32_t level)
{
	//写入LOG线程
	LogInfoData* data = LOGPOOL.pop_obj();
	if(data != NULL)
	{
		data->index = m_log_index;
		data->level = level;
		data->str_log = m_strm.str();
		
		LOG_THREAD_INSTANCE()->put_log(data);

		m_strm.str("");
	}
}

void BaseLogStream::set_log_name(const std::string& fileName)
{
	BaseLog* log = LOG_INSTANCE()->get_log_handler(m_log_index);
	if (log != NULL)
	{
		log->change_log_file(fileName);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
SingleLogStream::SingleLogStream(const char* pfile_name, int32_t level) : BaseLog(pfile_name)
{
	m_level = level;
}

SingleLogStream::~SingleLogStream()
{

}


std::ostream& SingleLogStream::dump_trace(int32_t _level)
{
	if(_level > 0 && _level < 5)
	{
		write_log(title_str[_level]);
		m_of_file << "\t";
	}

	return m_of_file;
}

std::ostream& SingleLogStream::get_ostream()
{
	return m_of_file;
}


void SingleLogStream::put_log(int32_t level)
{
}


