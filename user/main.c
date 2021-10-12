#include "rtdef.h"
#include "rtconfig.h"
#include "rtthread.h"
#include "scheduler.h"
#include "rthw.h"
#include "ARMCM7.h"

rt_uint8_t flag1;
rt_uint8_t flag2;

extern rt_list_t rt_thread_priority_table[RT_THREAD_PRIORITY_MAX];


ALIGN(RT_ALIGN_SIZE)
/* 定义线程栈 */
rt_uint8_t rt_flag1_thread_stack[512];
rt_uint8_t rt_flag2_thread_stack[512];

/* 定义线程控制块 */        				
struct rt_thread rt_flag1_thread;
struct rt_thread rt_flag2_thread;

void delay (uint32_t count);
void flag1_thread_entry (void *p_arg);
void flag2_thread_entry (void *p_arg);


int main (void)
{
    /* 硬件初始化 */
    
    /* 关中断 */
//    rt_hw_interrupt_disable();
    
//    SysTick_Config(SystemCoreClock / RT_TICK_PER_SECOND);

    /* 调度器初始化 */
    rt_system_scheduler_init();
    
    /* 初始化空闲线程 */
//    rt_thread_idle_init();
       
    
    /* 初始化线程 */
    rt_thread_init(&rt_flag1_thread,                /* 线程控制块 */
                    flag1_thread_entry,             /* 线程入口地址 */
                    RT_NULL,                        /* 线程形参 */
                   &rt_flag1_thread_stack[0],       /* 线程栈起始地址 */
                    sizeof(rt_flag1_thread_stack)); /* 线程栈大小，单位为字节 */
    
    /* 将线程插入到就绪列表中 */
    rt_list_insert_before(&(rt_thread_priority_table[0]), &(rt_flag1_thread.tlist));
    
    /* 初始化线程 */
    rt_thread_init(&rt_flag2_thread,                /* 线程控制块 */
                    flag2_thread_entry,             /* 线程入口地址 */
                    RT_NULL,                        /* 线程形参 */
                   &rt_flag2_thread_stack[0],       /* 线程栈起始地址 */
                    sizeof(rt_flag2_thread_stack)); /* 线程栈大小，单位为字节 */
    
    /* 将线程插入到就绪列表中 */
    rt_list_insert_before(&(rt_thread_priority_table[1]), &(rt_flag2_thread.tlist));
    
    /* 启动系统调度器 */
    rt_system_scheduler_start();
}

/* 软件延时 */
void delay (uint32_t count)
{
    while (count != 0) {
        count--;
    }
}

/* 线程1 */
void flag1_thread_entry (void *p_arg)
{
    for (; ;){
#if 1
        flag1 = 1;
        delay(100);
        flag1 = 0;
        delay(100);
        
        /* 线程切换，这里是手动切换 */
        rt_schedule();
#else
        flag1 = 1;
        rt_thread_delay(2);
        flag1 = 0;
        rt_thread_delay(2);
#endif
    }
}

/* 线程2 */
void flag2_thread_entry (void *p_arg)
{
    for (; ;){
#if 1
        flag2 = 1;
        delay(100);
        flag2 = 0;
        delay(100);
        
        /* 线程切换，这里是手动切换 */
        rt_schedule();
#else
        flag2 = 1;
        rt_thread_delay(2);
        flag2 = 0;
        rt_thread_delay(2);
#endif
    }
}


