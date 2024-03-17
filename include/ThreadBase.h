/*
 * File:   ThreadBase.h
 * Author: summer@ZMLAB
 * Date:   2022-08-17
 * Upadte: 2024-03-17
 * Description: General thread base
 */

#ifndef _THREAD_BASE_H_
#define _THREAD_BASE_H_

#include <unistd.h>
#include <string.h>
#include <pthread.h>


class ThreadBase
{
public:
    ThreadBase();
    ~ThreadBase();

    bool Start();                          // 开始执行线程
    virtual void Run() = 0;                // 线程运行实体 & 实现具体业务逻辑
    void Join();                           // 等待线程退出
    void Join(unsigned long millisTime);   // 等待线程退出 或者 超时
    
    pthread_t GetThreadID() {return m_ThreadID;};           // 获取线程ID
    int GetThreadStatus() {return m_ThreadStatus;};         // 获取线程状态
    void SetSelfTID(unsigned int tid) {m_SelfTID = tid;};   // 设置SelfTID
    unsigned int GetSelfTID() {return m_SelfTID;};          // 获取SelfTID
    // tips summer@20220818 - 注意指针判空 & 安全使用
    void SetSelfTName(const char* name);       // 设置SelfTName
    void GetSelfTName(char *name);             // 获取SelfTName
    void SetCpuAffinity(unsigned int cpuid);   // 设置CPU绑核

private:
    static void *thread_proxy_func(void *args);   // 获取线程执行方法的指针
    void *run();   // 线程内部执行方法

    pthread_t m_ThreadID;      // 线程ID (系统分配的ull类型)
    int m_ThreadStatus;        // 线程状态
    unsigned int m_SelfTID;    // 自定义ThreadID
    char m_SelfTName[128];     // 自定义Thread名字

/// 线程当前状态
public:
    static const int THREAD_STATUS_NEW     = 0;    // 新建
    static const int THREAD_STATUS_RUNNING = 1;    // 运行
    static const int THREAD_STATUS_EXIT    = -1;   // 结束
};

#endif   /*_THREAD_BASE_H_*/
