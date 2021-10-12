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
    
    /* ��ʼ���߳�ջ���������߳�ջָ�� */
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
//     * �̶߳����ʼ�� 
//     * �߳̽ṹ�忪ͷ���ֵ�4����Ա����rt_object_t��Ա 
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
//    /* ��ʼ���߳�ջ���������߳�ջָ�� */
//    thread->sp = 
//    (void *)rt_hw_stack_init(thread->entry,
//                             thread->parameter,
//    (void *)((char *)thread->stack_addr + thread->stack_size - 4));
//    
//    /* ��ʼ���߳����ȼ� */
//    thread->init_priority    = priority;
//    thread->current_priority = priority;
//    thread->number_mask = 0;
//    
//    /* �������״̬ */
//    thread->error = RT_EOK;
//    thread->stat  = RT_THREAD_INIT;
//    
//    return RT_EOK;
//}
#endif


void rt_thread_delay (rt_tick_t tick)
{
    struct rt_thread *thread;
    
    /* ��ȡ��ǰ�̵߳��߳̿��ƿ� */
    thread = rt_current_thread;
    
    /* ������ʱʱ�� */
    thread->remaining_tick = tick;
    
    /* ����ϵͳ���� */
    rt_schedule();
}

/**
 * �ú������ڻָ�һ���߳�Ȼ����ŵ������б�
 *
 * \param thread    ��Ҫ���ָ����߳�
 *
 * \return ���ز���״̬��RT_EOK on OK, -RT_ERROR on error
 */
rt_err_t rt_thread_resume (rt_thread_t thread)
{
    register rt_base_t temp;
    
    /* �����ָ����̱߳��봦�ڹ���̬�����򷵻ش����� */
    if ((thread->stat & RT_THREAD_STAT_MASK) != RT_THREAD_SUSPEND) {
        return -RT_ERROR;
    }
    
    /* ���ж� */
    temp = rt_hw_interrupt_disable();
    
    /* �ӹ�������Ƴ� */
    rt_list_remove(&(thread->tlist));
    
    /* ���ж� */
    rt_hw_interrupt_enable(temp);
    
    /* ��������б� */
    rt_schedule_insert_thread(thread);
    
    return RT_EOK;
}

rt_thread_t rt_thread_self (void)
{
    return rt_current_thread;
}

/**
 * ����һ���̲߳�����ŵ�ϵͳ�ľ����б���
 *
 * \param thread    ���������߳�
 * 
 * \return ���ز���״̬��RT_EOK on OK, -RT_ERROR on error
 */
rt_err_t rt_thread_startup (rt_thread_t thread)
{
    /* ���õ�ǰ���ȼ�Ϊ��ʼ���ȼ� */
    thread->current_priority = thread->init_priority;
    thread->number_mask = 1L << thread->current_priority;
    
    /* �ı��̵߳�״̬Ϊ����״̬ */
    thread->stat = RT_THREAD_SUSPEND;
    /* Ȼ��ָ��߳� */
    rt_thread_resume(thread);
    
    if (rt_thread_self() != RT_NULL) {
        /* ϵͳ���� */
        rt_schedule();
    }
    return RT_EOK;
}


