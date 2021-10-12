#include "rtthread.h"
#include "rthw.h"
#include "scheduler.h"

#define IDLE_THREAD_STACK_SIZE      512

ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t rt_thread_stack[IDLE_THREAD_STACK_SIZE];

/* �����̵߳��߳̿��ƿ� */
struct rt_thread idle;

rt_ubase_t rt_idletask_ctr = 0;

void rt_thread_idle_entry (void *parameter)
{
    parameter = parameter;
    while (1) {
        rt_idletask_ctr++;
    }
}

//void rt_thread_idle_init (void)
//{
//    /* ��ʼ���߳� */
//    rt_thread_init(&idle,
//                   "idle",
//                    rt_thread_idle_entry,
//                    RT_NULL,
//                   &rt_thread_stack[0],
//                    sizeof(rt_thread_stack),
//                    RT_THREAD_PRIORITY_MAX-1);
//    
//    /* ���̲߳��뵽�����б� */
////    rt_list_insert_before(&(rt_thread_priority_table[RT_THREAD_PRIORITY_MAX-1]),
////                          &(idle.list));
//    rt_thread_startup(&idle);
//}

