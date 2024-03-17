/*
 * File:   ThreadBase.cpp
 * Author: summer@ZMLAB
 * Date:   2022-08-17
 * Upadte: 2024-03-17
 * Description: General thread base
 */

#include <iostream>
#include "ThreadBase.h"


ThreadBase::ThreadBase()
{  
    m_ThreadID = 0;
    m_ThreadStatus = THREAD_STATUS_NEW;
    m_SelfTID = 0;
    memset(m_SelfTName, 0, 128);
}

ThreadBase::~ThreadBase()
{  
    //
}

//##################################################//
//   启动线程运行实体
//##################################################//
bool ThreadBase::Start()
{
    return pthread_create(&m_ThreadID, NULL, thread_proxy_func, this);
}

//##################################################//
//   等待线程退出
//##################################################//
void ThreadBase::Join()
{  
    if( m_ThreadID > 0 )
    {
        pthread_join(m_ThreadID, NULL);
    }
}

//##################################################//
//   等待线程退出 或者 超时
//##################################################//
void ThreadBase::Join(unsigned long millisTime)
{  
    if( m_ThreadID == 0 )
    {
        return;
    }

    if( millisTime == 0 )  
    {  
        Join();
    }
    else
    {
        unsigned long k = 0;  
        while( (m_ThreadStatus != THREAD_STATUS_EXIT) && (k <= millisTime) )
        {
            usleep(100);
            k++;
        }
    }
}

//##################################################//
//   设置SelfTName
//##################################################//
void ThreadBase::SetSelfTName(const char *name)
{
    memset(m_SelfTName, 0, 128);
    strcpy(m_SelfTName, name);
    
    //@Linux//
    // 先run才能有效设置线程名称
    // 如果pid的值为0
    // 则表示指定的是当前进程 
    if( m_ThreadID > 0 )
    {
        //@Linux// name保持在16字节以内
        pthread_setname_np(m_ThreadID, name);
    }
}

//##################################################//
//   获取SelfTName
//##################################################//
void ThreadBase::GetSelfTName(char *name)
{
    strcpy(name, m_SelfTName);
}

//##################################################//
//   设置CPU绑核
//##################################################//
void ThreadBase::SetCpuAffinity(unsigned int cpuid)
{
    if( m_ThreadID > 0 )
    {
        cpu_set_t mask;
        // 掩码清零
        CPU_ZERO(&mask);
        // 将cpuid添加到掩码中
        CPU_SET(cpuid, &mask);
        // 将本线程CPU绑核
        pthread_setaffinity_np(m_ThreadID, sizeof(cpu_set_t), &mask);
    }
}

void *ThreadBase::thread_proxy_func(void *args)
{
    ThreadBase *pThread = static_cast<ThreadBase *>(args);
    pThread->run();
    return NULL;
}

void *ThreadBase::run()
{
    m_ThreadStatus = THREAD_STATUS_RUNNING;
    m_ThreadID = pthread_self();

    Run();

    m_ThreadStatus = THREAD_STATUS_EXIT;
    m_ThreadID = 0;

    pthread_exit(NULL);
}

//----------------------------------------//
//-----    River flows in summer     -----//
//----------------------------------------//