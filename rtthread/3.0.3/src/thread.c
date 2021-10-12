#include "rtthread.h"
#include "cpuport.h"
#include "scheduler.h"
#include "rthw.h"

#if 1
rt_err_t rt_thread_init (struct rt_thread *thread,
                         void (*entry) (void *parameter),
                         void             *parameter,
                         void             *stack_start,
                         rt_uint32_t       stack_size)
{
    rt_list_init(&(thread->list));
    
    thread->entry = (void *)entry;
    thread->parameter = parameter;
    
    thread->stack_addr = stack_start;
    thread->stack_size = stack_size;
    
    /* 初始化线程栈，并返回线程栈指针 */
    thread->sp = 
    (void *)rt_hw_stack_init(thread->entry,
                             thread->parameter,
    (void *)((char *)thread->stack_addr + thread->stack_size - 4));
    
    return RT_EOK;
}
#else
//rt_err_t rt_thread_init (struct rt_thread *thread,
//                         const char       *name,
//                         void (*entry) (void *parameter),
//                         void             *parameter,
//                         void             *stack_start,
//                         rt_uint32_t       stack_size,
//                         rt_uint8_t        priority)
//{
//    /* 
//     * 线程对象初始化 
//     * 线程结构体开头部分的4个成员就是rt_object_t成员 
//     */
//    rt_object_init((rt_object_t)thread, RT_Object_Class_Thread, name);
//    rt_list_init(&(thread->tlist));
//    
//    thread->entry = (void *)entry;
//    thread->parameter = parameter;
//    
//    thread->stack_addr = stack_start;
//    thread->stack_size = stack_size;
//    
//    /* 初始化线程栈，并返回线程栈指针 */
//    thread->sp = 
//    (void *)rt_hw_stack_init(thread->entry,
//                             thread->parameter,
//    (void *)((char *)thread->stack_addr + thread->stack_size - 4));
//    
//    /* 初始化线程优先级 */
//    thread->init_priority    = priority;
//    thread->current_priority = priority;
//    thread->number_mask = 0;
//    
//    /* 错误码和状态 */
//    thread->error = RT_EOK;
//    thread->stat  = RT_THREAD_INIT;
//    
//    return RT_EOK;
//}
#endif


void rt_thread_delay (rt_tick_t tick)
{
    struct rt_thread *thread;
    
    /* 获取当前线程的线程控制块 */
    thread = rt_current_thread;
    
    /* 设置延时时间 */
    thread->remaining_tick = tick;
    
    /* 进行系统调度 */
    rt_schedule();
}

/**
 * 该函数用于恢复一个线程然后将其放到就绪列表
 *
 * \param thread    需要被恢复的线程
 *
 * \return 返回操作状态，RT_EOK on OK, -RT_ERROR on error
 */
rt_err_t rt_thread_resume (rt_thread_t thread)
{
    register rt_base_t temp;
    
    /* 将被恢复的线程必须处于挂起态，否则返回错误码 */
    if ((thread->stat & RT_THREAD_STAT_MASK) != RT_THREAD_SUSPEND) {
        return -RT_ERROR;
    }
    
    /* 关中断 */
    temp = rt_hw_interrupt_disable();
    
    /* 从挂起队列移除 */
    rt_list_remove(&(thread->tlist));
    
    /* 开中断 */
    rt_hw_interrupt_enable(temp);
    
    /* 插入就绪列表 */
    rt_schedule_insert_thread(thread);
    
    return RT_EOK;
}

rt_thread_t rt_thread_self (void)
{
    return rt_current_thread;
}

/**
 * 启动一个线程并将其放到系统的就绪列表中
 *
 * \param thread    待启动的线程
 * 
 * \return 返回操作状态，RT_EOK on OK, -RT_ERROR on error
 */
rt_err_t rt_thread_startup (rt_thread_t thread)
{
    /* 设置当前优先级为初始优先级 */
    thread->current_priority = thread->init_priority;
    thread->number_mask = 1L << thread->current_priority;
    
    /* 改变线程的状态为挂起状态 */
    thread->stat = RT_THREAD_SUSPEND;
    /* 然后恢复线程 */
    rt_thread_resume(thread);
    
    if (rt_thread_self() != RT_NULL) {
        /* 系统调度 */
        rt_schedule();
    }
    return RT_EOK;
}



