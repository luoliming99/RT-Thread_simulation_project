#include "scheduler.h"
#include "rthw.h"
#include "rtdef.h"
#include "rtthread.h"
    
/* �߳̾����б� */
rt_list_t rt_thread_priority_table[RT_THREAD_PRIORITY_MAX];

/* �߳̿��ƿ�ȫ��ָ�� */
struct rt_thread *rt_current_thread;

/* ��ǰ�����̵߳����ȼ� */
rt_uint8_t rt_current_priority;

/* �߳̾������ȼ��� */
rt_uint32_t rt_thread_ready_priority_group;


/* ��ʼ��ϵͳ������ */
void rt_system_scheduler_init (void)
{
    register rt_base_t offset;
    
    /* �߳����ȼ����ʼ�� */
    for (offset = 0; offset < RT_THREAD_PRIORITY_MAX; offset++) {
        rt_list_init(&rt_thread_priority_table[offset]);
    }
    
    /* ��ʼ����ǰ���ȼ�Ϊ�����̵߳����ȼ� */
    rt_current_priority = RT_THREAD_PRIORITY_MAX - 1;
    
    /* ��ʼ����ǰ�߳̿��ƿ�ָ�� */
    rt_current_thread = RT_NULL;
    
    /* ��ʼ���߳̾������ȼ��� */ 
    rt_thread_ready_priority_group = 0;
}

/* ����ϵͳ������ */
void rt_system_scheduler_start (void)
{
    register struct rt_thread *to_thread;
    register rt_base_t highest_ready_priority;
    
    /* ��ȡ������������ȼ� */
    highest_ready_priority = __rt_ffs(rt_thread_ready_priority_group) - 1;
    
    /* ��ȡ��Ҫ�����̵߳��߳̿��ƿ� */
    to_thread = rt_list_entry(rt_thread_priority_table[highest_ready_priority].next,
                              struct rt_thread,
                              tlist);
    rt_current_thread = to_thread;
    
    /* �л����µ��߳� */
    rt_hw_context_switch_to((rt_uint32_t)&to_thread->sp);
                              
    /* ��Զ���᷵�� */ 
    to_thread = to_thread;
}

/* ϵͳ���� */
void rt_schedule (void)
{   
    register struct rt_thread *to_thread;
    register struct rt_thread *from_thread;
    register rt_base_t highest_ready_priority;
    register rt_base_t level;
    
    /* ���ж� */
    level = rt_hw_interrupt_disable();
    /* ��ȡ������������ȼ� */
    highest_ready_priority = __rt_ffs(rt_thread_ready_priority_group) - 1;
    /* ��ȡ��Ҫ�����̵߳��߳̿��ƿ� */
    to_thread = rt_list_entry(rt_thread_priority_table[highest_ready_priority].next,
                              struct rt_thread,
                              tlist);
    /* ���ж� */
    rt_hw_interrupt_enable(level);
                              
    /* ���Ŀ���̲߳��ǵ�ǰ�̣߳���Ҫ�����߳��л� */
    if (to_thread != rt_current_thread) {
        rt_current_priority = (rt_uint8_t)highest_ready_priority;
        from_thread         = rt_current_thread;
        rt_current_thread   = to_thread;
        rt_hw_context_switch((rt_uint32_t)&from_thread->sp, (rt_uint32_t)&to_thread->sp);
        /* �����Ƿ��ִ������ */
        to_thread = to_thread;
    }
}

void rt_schedule_insert_thread (struct rt_thread *thread)
{
    register rt_base_t temp;
    
    /* ���ж� */
    temp = rt_hw_interrupt_disable();
    
    /* �ı��߳�״̬ */
    thread->stat = RT_THREAD_READY;
    
    /* ���̲߳�������б� */
    rt_list_insert_before(&(rt_thread_priority_table[thread->current_priority]),
                          &(thread->tlist));
    
    /* �����߳̾������ȼ����ж�Ӧ��λ */
    rt_thread_ready_priority_group |= thread->number_mask;
    
    /* ���ж� */
    rt_hw_interrupt_enable(temp);
}

void rt_schedule_remove_thread (struct rt_thread *thread)
{
    register rt_base_t temp;
      
    /* ���ж� */
    temp = rt_hw_interrupt_disable();
    
    /* ���̴߳Ӿ����б�ɾ�� */
    rt_list_remove(&(thread->tlist));
    /* ���߳̾������ȼ����Ӧ��λ��� */
    if (rt_list_isempty(&(rt_thread_priority_table[thread->current_priority])))
    {
        rt_thread_ready_priority_group &= ~thread->number_mask;
    }
    
    /* ���ж� */
    rt_hw_interrupt_enable(temp);
}



