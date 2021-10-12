#include "scheduler.h"
#include "rthw.h"
#include "rtdef.h"
    
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
#if 1
    register rt_base_t offset;
    
    /* �߳̾����б��ʼ�� */
    for (offset = 0; offset < RT_THREAD_PRIORITY_MAX; offset++) {
        rt_list_init(&rt_thread_priority_table[offset]);
    }
    
    /* ��ʼ����ǰ�߳̿��ƿ�ָ�� */
    rt_current_thread = RT_NULL;
#else
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
#endif
}

extern struct rt_thread rt_flag1_thread;
extern struct rt_thread rt_flag2_thread;
extern struct rt_thread idle;

/* ����ϵͳ������ */
void rt_system_scheduler_start (void)
{
    register struct rt_thread *to_thread;
    
    
    /* �ֶ�ָ����һ�����е��߳� */
    to_thread = rt_list_entry(rt_thread_priority_table[0].next,
                              struct rt_thread,
                              tlist);
                             
    rt_current_thread = to_thread;
          
    /* �л�����һ���̣߳��ú�����context_rvds.S��ʵ�֣�
       ��rthw.h����������ʵ�ֵ�һ���߳��л���
       ��һ����ຯ����C�ļ��е��õ�ʱ��������βΣ�
       ��ִ�е�ʱ��Ὣ�βδ��뵽CPU�Ĵ���r0�� */
    rt_hw_context_switch_to((rt_uint32_t)&to_thread->sp);                            
}



/* ϵͳ���� */
void rt_schedule (void)
{   
    register struct rt_thread *to_thread;
    register struct rt_thread *from_thread;
   
#if 1    
    /* �����߳������л� */
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
     
    /* �����������л� */
    rt_hw_context_switch((rt_uint32_t)&from_thread->sp, (rt_uint32_t)&to_thread->sp);
#else
    
    /* 
     * �����ǰ�߳��ǿ����̣߳���ô��ȥ����ִ���߳�1�����߳�2���������ǵ���ʱʱ
     * ���Ƿ����������̵߳���ʱʱ���û�е��ڣ��Ǿͷ��ؼ���ִ�п����߳� 
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
    } else {    /* ��ǰ�̲߳��ǿ����߳����ִ�е����� */
        /* 
         * �����ǰ�߳����߳�1�����߳�2�Ļ������������һ���̣߳����������߳�
         * ������ʱ�У����л������̣߳������ж��µ�ǰ�߳��Ƿ�Ӧ�ý�����ʱ״̬��
         * ����ǵĻ������л��������̣߳�����Ͳ������κ��л�
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
                return;     /* ���أ��������л�����Ϊ�߳�1��������ʱ���߳�2������ʱ */
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
                return;     /* ���أ��������л�����Ϊ�߳�2��������ʱ���߳�1������ʱ */
            }
        }
    }
#endif
    /* �����������л� */
    rt_hw_context_switch((rt_uint32_t)&from_thread->sp, (rt_uint32_t)&to_thread->sp);
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


