/********************************************************
function: log class
version:  1.0
date:     2013.04.19
modify:   2024.03.18
*********************************************************/

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <pthread.h>
#include "log.h"

using namespace std;

Log::Log()
{
	m_count = 0;
	m_mutex = new pthread_mutex_t;
	pthread_mutex_init(m_mutex, NULL);
}

Log::~Log()
{
	if(m_fp != NULL)
	{
		fclose(m_fp);
	}
	pthread_mutex_destroy(m_mutex);
	if(m_mutex != NULL)
	{
		delete m_mutex;
	}
}

bool Log::init(const char* file_name, int log_buf_size, int split_lines)
{
	m_log_buf_size = log_buf_size;
	m_buf = new char[m_log_buf_size];
	// debug summer@20221005
	// sizeof(m_buf) == 4
	//memset(m_buf, '\0', sizeof(m_buf));
	memset(m_buf, '\0', m_log_buf_size);
	m_split_lines = split_lines;

	time_t t = time(NULL);
	struct tm* sys_tm = localtime(&t);
	struct tm my_tm = *sys_tm;
	char *p = (char*)strrchr(file_name, '/');
	char log_full_name[512] = {0};
	if(p == NULL)
	{
		snprintf(log_full_name, 511, "%d_%02d_%02d_%s",my_tm.tm_year+1900, my_tm.tm_mon+1, my_tm.tm_mday, file_name);	
	}
	else
	{
		strcpy(log_name, p+1);
		strncpy(dir_name, file_name, p - file_name + 1);
		snprintf(log_full_name, 511, "%s%d_%02d_%02d_%s", dir_name, my_tm.tm_year+1900, my_tm.tm_mon+1, my_tm.tm_mday, log_name); 
	}

	m_today = my_tm.tm_mday;

	m_fp = fopen(log_full_name, "a");
	if(m_fp == NULL)
	{
		return false;
	}

	return true;
}

void Log::write_log(int level, const char* format, ...)
{
	memset(m_buf, '\0', m_log_buf_size);
	struct timeval now = {0,0};
	gettimeofday(&now, NULL);
	time_t t = now.tv_sec;
	struct tm* sys_tm = localtime(&t);
	struct tm my_tm = *sys_tm;
	char s[16] = {0};
	switch(level)
	{
		case 0 : strcpy(s, "[DEBUG]:"); break;
		case 1 : strcpy(s, "[INFO]:"); break;
		case 2 : strcpy(s, "[WARN]:"); break;
		case 3 : strcpy(s, "[ERROR]:"); break;
		default:
			     strcpy(s, "[INFO]:"); break;
	}
	// debug summer@20221005 - Type Cast
	// int n = snprintf(m_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06d %s ",
	// 		my_tm.tm_year+1900, my_tm.tm_mon+1, my_tm.tm_mday,
	// 		my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, s);
	int n = snprintf(m_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06d %s ",
			my_tm.tm_year+1900, my_tm.tm_mon+1, my_tm.tm_mday,
			my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, (int)now.tv_usec, s);

	pthread_mutex_lock(m_mutex);
	m_count++;
	// everyday log
	if(m_today != my_tm.tm_mday || m_count % m_split_lines == 0)
	{
		char new_log[512] = {0};
		fflush(m_fp);
		fclose(m_fp);
		char tail[16] = {0};
		snprintf(tail, 16,  "%d_%02d_%02d_", my_tm.tm_year+1900, my_tm.tm_mon+1, my_tm.tm_mday);
		if(m_today != my_tm.tm_mday)
		{
			snprintf(new_log, 511, "%s%s%s", dir_name, tail, log_name);
			m_today = my_tm.tm_mday;
			m_count = 0;
		}
		else
		{
			// debug summer@20221005 - Type Cast
			//snprintf(new_log, 255, "%s%s%s.%d", dir_name, tail, log_name, m_count/m_split_lines);
			snprintf(new_log, 511, "%s%s%s.%lld", dir_name, tail, log_name, m_count/m_split_lines);
		}
		m_fp = fopen(new_log, "a");
	}
	pthread_mutex_unlock(m_mutex);
	
	va_list valst;
	va_start(valst, format);

	pthread_mutex_lock(m_mutex);
	int m = vsnprintf(m_buf + n, m_log_buf_size-1, format, valst);
	m_buf[n + m ] = '\n';
	//std::cout<<"log:"<<m_buf<<std::endl;
	fputs(m_buf, m_fp);
	pthread_mutex_unlock(m_mutex);

	va_end(valst);
	flush();
}

void Log::flush(void)
{
	pthread_mutex_lock(m_mutex);
	fflush(m_fp);
	pthread_mutex_unlock(m_mutex);
}
