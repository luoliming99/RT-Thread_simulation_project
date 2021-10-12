#include "rtthread.h"
#include "rthw.h"
#include "scheduler.h"

/* �жϼ����� */
volatile rt_uint8_t rt_interrupt_nest;


/**
 * ��BSP�ļ����жϷ���������ʱ����øú���
 * 
 * @note �벻Ҫ��Ӧ�ó����е��øú���
 *
 * @see rt_interrupt_enter
 */
void rt_interrupt_enter (void)
{
    rt_base_t level;
    
    /* ���ж� */
    level = rt_hw_interrupt_disable();
    
    /* �жϼ�����++ */
    rt_interrupt_nest++;
    
    /* ���ж� */
    rt_hw_interrupt_enable(level);
}

/**
 * ��BSP�ļ����жϷ������뿪ʱ����øú���
 * 
 * @note �벻Ҫ��Ӧ�ó����е��øú���
 *
 * @see rt_interrupt_leave
 */
void rt_interrupt_leave (void)
{
    rt_base_t level;
    
    /* ���ж� */
    level = rt_hw_interrupt_disable();
    
    /* �жϼ�����-- */
    rt_interrupt_nest--;
    
    /* ���ж� */
    rt_hw_interrupt_enable(level);
}

static rt_tick_t rt_tick = 0;   /* ϵͳʱ�������� */
extern rt_list_t rt_thread_priority_table[RT_THREAD_PRIORITY_MAX];

void rt_tick_increase (void)
{
    rt_base_t i;
    struct rt_thread *thread;
    rt_tick++;
    
    /* ɨ������б��������̵߳�remaining_tick�������Ϊ0�����1 */
    for (i = 0; i < RT_THREAD_PRIORITY_MAX; i++) {
        thread = rt_list_entry(rt_thread_priority_table[i].next,
                               struct rt_thread,
                               tlist);
        if (thread->remaining_tick > 0) {
            thread->remaining_tick--;
        }
    }
    
    /* ϵͳ���� */
    rt_schedule();
}

void SysTick_Handler(void)
{
    /* �����ж� */
    rt_interrupt_enter();
    /* ʱ������ */
    rt_tick_increase();
    /* �뿪�ж� */
    rt_interrupt_leave();
}

