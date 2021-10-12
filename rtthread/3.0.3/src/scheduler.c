#include "scheduler.h"
#include "rthw.h"
#include "rtdef.h"
    
/* 线程就绪列表 */
rt_list_t rt_thread_priority_table[RT_THREAD_PRIORITY_MAX];

/* 线程控制块全局指针 */
struct rt_thread *rt_current_thread;

/* 当前运行线程的优先级 */
rt_uint8_t rt_current_priority;

/* 线程就绪优先级组 */
rt_uint32_t rt_thread_ready_priority_group;


/* 初始化系统调度器 */
void rt_system_scheduler_init (void)
{
#if 1
    register rt_base_t offset;
    
    /* 线程就绪列表初始化 */
    for (offset = 0; offset < RT_THREAD_PRIORITY_MAX; offset++) {
        rt_list_init(&rt_thread_priority_table[offset]);
    }
    
    /* 初始化当前线程控制块指针 */
    rt_current_thread = RT_NULL;
#else
    register rt_base_t offset;
    
    /* 线程优先级表初始化 */
    for (offset = 0; offset < RT_THREAD_PRIORITY_MAX; offset++) {
        rt_list_init(&rt_thread_priority_table[offset]);
    }
    
    /* 初始化当前优先级为空闲线程的优先级 */
    rt_current_priority = RT_THREAD_PRIORITY_MAX - 1;
    
    /* 初始化当前线程控制块指针 */
    rt_current_thread = RT_NULL;
    
    /* 初始化线程就绪优先级组 */
    rt_thread_ready_priority_group = 0;
#endif
}

extern struct rt_thread rt_flag1_thread;
extern struct rt_thread rt_flag2_thread;
extern struct rt_thread idle;

/* 启动系统调度器 */
void rt_system_scheduler_start (void)
{
    register struct rt_thread *to_thread;
    
    
    /* 手动指定第一个运行的线程 */
    to_thread = rt_list_entry(rt_thread_priority_table[0].next,
                              struct rt_thread,
                              tlist);
                             
    rt_current_thread = to_thread;
          
    /* 切换到第一个线程，该函数在context_rvds.S中实现，
       在rthw.h声明，用于实现第一次线程切换。
       当一个汇编函数在C文件中调用的时候，如果有形参，
       则执行的时候会将形参传入到CPU寄存器r0。 */
    rt_hw_context_switch_to((rt_uint32_t)&to_thread->sp);                            
}



/* 系统调度 */
void rt_schedule (void)
{   
    register struct rt_thread *to_thread;
    register struct rt_thread *from_thread;
   
#if 1    
    /* 两个线程轮流切换 */
    if (rt_current_thread == rt_list_entry(rt_thread_priority_table[0].next,
                                           struct rt_thread,
                                           tlist)) {
        from_thread = rt_current_thread;
        to_thread = rt_list_entry(rt_thread_priority_table[1].next,
                                  struct rt_thread,
                                  tlist);
        rt_current_thread = to_thread;
    } else {
        from_thread = rt_current_thread;
        to_thread = rt_list_entry(rt_thread_priority_table[0].next,
                                  struct rt_thread,
                                  tlist);
        rt_current_thread = to_thread;
    }
     
    /* 产生上下文切换 */
    rt_hw_context_switch((rt_uint32_t)&from_thread->sp, (rt_uint32_t)&to_thread->sp);
#else
    
    /* 
     * 如果当前线程是空闲线程，那么就去尝试执行线程1或者线程2，看看他们的延时时
     * 间是否结束，如果线程的延时时间均没有到期，那就返回继续执行空闲线程 
     */
    if (rt_current_thread == &idle) {
        if (rt_flag1_thread.remaining_tick == 0) {
            from_thread       =  rt_current_thread;
            to_thread         = &rt_flag1_thread;
            rt_current_thread =  to_thread;  
        } else if (rt_flag2_thread.remaining_tick == 0) {
            from_thread       =  rt_current_thread;
            to_thread         = &rt_flag2_thread;
            rt_current_thread =  to_thread;
        } else {
            return;
        }
    } else {    /* 当前线程不是空闲线程则会执行到这里 */
        /* 
         * 如果当前线程是线程1或者线程2的话，检查下另外一个线程，如果另外的线程
         * 不在延时中，就切换到该线程；否则，判断下当前线程是否应该进入延时状态，
         * 如果是的话，就切换到空闲线程，否则就不进行任何切换
         */
        if (rt_current_thread == &rt_flag1_thread) {
            if (rt_flag2_thread.remaining_tick == 0) {
                from_thread       =  rt_current_thread;
                to_thread         = &rt_flag2_thread;
                rt_current_thread =  to_thread;
            } else if (rt_current_thread->remaining_tick != 0) {
                from_thread       =  rt_current_thread;
                to_thread         = &idle;
                rt_current_thread =  to_thread;
            } else {
                return;     /* 返回，不进行切换，因为线程1不处于延时，线程2处于延时 */
            }
        } else if (rt_current_thread == &rt_flag2_thread) {
            if (rt_flag1_thread.remaining_tick == 0) {
                from_thread       =  rt_current_thread;
                to_thread         = &rt_flag1_thread;
                rt_current_thread =  to_thread;
            } else if (rt_current_thread->remaining_tick != 0) {
                from_thread       =  rt_current_thread;
                to_thread         = &idle;
                rt_current_thread =  to_thread;
            } else {
                return;     /* 返回，不进行切换，因为线程2不处于延时，线程1处于延时 */
            }
        }
    }
#endif
    /* 产生上下文切换 */
    rt_hw_context_switch((rt_uint32_t)&from_thread->sp, (rt_uint32_t)&to_thread->sp);
}

void rt_schedule_insert_thread (struct rt_thread *thread)
{
    register rt_base_t temp;
    
    /* 关中断 */
    temp = rt_hw_interrupt_disable();
    
    /* 改变线程状态 */
    thread->stat = RT_THREAD_READY;
    
    /* 将线程插入就绪列表 */
    rt_list_insert_before(&(rt_thread_priority_table[thread->current_priority]),
                          &(thread->tlist));
    
    /* 设置线程就绪优先级组中对应的位 */
    rt_thread_ready_priority_group |= thread->number_mask;
    
    /* 开中断 */
    rt_hw_interrupt_enable(temp);
}


