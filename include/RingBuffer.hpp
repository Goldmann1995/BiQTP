/*
 * File:        RingBuffer.hpp
 * Author:      summer@ZMLAB
 * CreateDate:  2022-08-28
 * LastEdit:    2024-03-18
 * Description: RingBuffer适用于HF大容量数据传输
 *              本例RB并非线程安全 必须遵循一个线程write 另一线程read的原则
 *              本例RB并非100%可靠 write速度>read速度会发生丢包
 */

#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#include <strings.h>
#include <utility>

#define RING_BUFFER_SIZE   (1024*16)


//--------------------------------------------------//
//@RB// first == true  // FULL  -> readable
//@RB// first == false // EMPTY -> writable
//--------------------------------------------------//
template <class T>
class RingBuffer
{
public:
    //########################################//
    //   Constructor
    //########################################//
    RingBuffer(): read_index(0), write_index(0), aggr_cnt(0), loss_cnt(0)
    {
        for(int i=0; i<RING_BUFFER_SIZE; i++)
        {
            msg_loop[i].first = false;
            //@C++// 不能memset非POD类型
            //memset(&msg_loop[i].second, 0, sizeof(T));
            //@C++// 替换placement new
            new (&msg_loop[i].second) T();
        }
    }

    //########################################//
    //   Destructor
    //########################################//
    ~RingBuffer()
    {
        // think summer@20230227
        // need memory free ?
    }

    //########################################//
    //   返回/判空 (可读)首指针
    //########################################//
    T *front()
    {
        if(msg_loop[read_index].first)
            return &msg_loop[read_index].second;
        else
            return nullptr;
    }

    //########################################//
    //   出队操作
    //########################################//
    void pop()
    {
        msg_loop[read_index].first = false;

        ++read_index;
        read_index = read_index % RING_BUFFER_SIZE;
    }

    //########################################//
    //   入队操作
    //########################################//
    bool push(const T &tmp_msg)
    {
        if( !msg_loop[write_index].first )
        {
            //--------------------------------------------------//
            //@C++// class T 务必进行赋值运算符重载 !!!
            //--------------------------------------------------//
            msg_loop[write_index].second = tmp_msg;
            msg_loop[write_index].first = true;

            ++write_index;
            write_index = write_index % RING_BUFFER_SIZE;

            aggr_cnt++;
            return true;
        }
        else
        {
            loss_cnt++;
            return false;
        }
    }

    //########################################//
    //   返回 未处理数据
    //########################################//
    int size()
    {
        return (write_index-read_index);
    }

    //########################################//
    //   返回 总包统计
    //########################################//
    int aggr()
    {
        return aggr_cnt;
    }

    //########################################//
    //   返回 丢包统计
    //########################################//
    int loss()
    {
        return loss_cnt;
    }

private:
    //--------------------------------------------------//
    //@C++// volatile无法保证操作的原子性
    //@C++// volatile强制内存访问 不需要cache优化
    //@C++// 防止HF中出现cache/memory同步一致性问题
    //--------------------------------------------------//
    std::pair<volatile bool, T> msg_loop[RING_BUFFER_SIZE];

    // 读&写指针
    unsigned int read_index;
    unsigned int write_index;

    // 总包统计
    unsigned int aggr_cnt;
    // 丢包统计
    unsigned int loss_cnt;
};

#endif   /*_RING_BUFFER_H_*/
