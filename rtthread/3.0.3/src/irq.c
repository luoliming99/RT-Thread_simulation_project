#include "rtthread.h"
#include "rthw.h"
#include "scheduler.h"

/* 中断计数器 */
volatile rt_uint8_t rt_interrupt_nest;


/**
 * 当BSP文件的中断服务函数进入时会调用该函数
 * 
 * @note 请不要在应用程序中调用该函数
 *
 * @see rt_interrupt_enter
 */
void rt_interrupt_enter (void)
{
    rt_base_t level;
    
    /* 关中断 */
    level = rt_hw_interrupt_disable();
    
    /* 中断计数器++ */
    rt_interrupt_nest++;
    
    /* 开中断 */
    rt_hw_interrupt_enable(level);
}

/**
 * 当BSP文件的中断服务函数离开时会调用该函数
 * 
 * @note 请不要在应用程序中调用该函数
 *
 * @see rt_interrupt_leave
 */
void rt_interrupt_leave (void)
{
    rt_base_t level;
    
    /* 关中断 */
    level = rt_hw_interrupt_disable();
    
    /* 中断计数器-- */
    rt_interrupt_nest--;
    
    /* 开中断 */
    rt_hw_interrupt_enable(level);
}

static rt_tick_t rt_tick = 0;   /* 系统时基计数器 */
extern rt_list_t rt_thread_priority_table[RT_THREAD_PRIORITY_MAX];

void rt_tick_increase (void)
{
    rt_base_t i;
    struct rt_thread *thread;
    rt_tick++;
    
    /* 扫描就绪列表中所有线程的remaining_tick，如果不为0，则减1 */
    for (i = 0; i < RT_THREAD_PRIORITY_MAX; i++) {
        thread = rt_list_entry(rt_thread_priority_table[i].next,
                               struct rt_thread,
                               tlist);
        if (thread->remaining_tick > 0) {
            thread->remaining_tick--;
        }
    }
    
    /* 系统调度 */
    rt_schedule();
}

void SysTick_Handler(void)
{
    /* 进入中断 */
    rt_interrupt_enter();
    /* 时基更新 */
    rt_tick_increase();
    /* 离开中断 */
    rt_interrupt_leave();
}

