/********************************************************
function: log class
version:  1.0
date:     2013.4.19
modify:   2024.03.18
*********************************************************/

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <iostream>
#include <stdarg.h>
#include <pthread.h>

using namespace std;

class Log
{
	public:
		// 单例模式
		static Log* get_instance()
		{
			static Log instance;
			return &instance;
		}

		bool init(const char* file_name, int log_buf_size = 8192, int split_lines = 5000000);

		void write_log(int level, const char* format, ...);

		void flush(void);

	private:
		Log();
		virtual ~Log();

	private:
		pthread_mutex_t *m_mutex;
		char dir_name[128];
		char log_name[128];
		int m_split_lines;
		int m_log_buf_size;
		long long  m_count;
		int m_today;
		FILE *m_fp;
		char *m_buf;
};

#define LOG_DEBUG(format, ...) Log::get_instance()->write_log(0, format, __VA_ARGS__)
#define LOG_INFO(format, ...) Log::get_instance()->write_log(1, format, __VA_ARGS__)
#define LOG_WARN(format, ...) Log::get_instance()->write_log(2, format, __VA_ARGS__)
#define LOG_ERROR(format, ...) Log::get_instance()->write_log(3, format, __VA_ARGS__)

#endif 