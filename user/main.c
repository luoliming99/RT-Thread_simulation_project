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
/* �����߳�ջ */
rt_uint8_t rt_flag1_thread_stack[512];
rt_uint8_t rt_flag2_thread_stack[512];

/* �����߳̿��ƿ� */        				
struct rt_thread rt_flag1_thread;
struct rt_thread rt_flag2_thread;

void delay (uint32_t count);
void flag1_thread_entry (void *p_arg);
void flag2_thread_entry (void *p_arg);


int main (void)
{
    /* Ӳ����ʼ�� */
    
    /* ���ж� */
//    rt_hw_interrupt_disable();
    
//    SysTick_Config(SystemCoreClock / RT_TICK_PER_SECOND);

    /* ��������ʼ�� */
    rt_system_scheduler_init();
    
    /* ��ʼ�������߳� */
//    rt_thread_idle_init();
       
    
    /* ��ʼ���߳� */
    rt_thread_init(&rt_flag1_thread,                /* �߳̿��ƿ� */
                    flag1_thread_entry,             /* �߳���ڵ�ַ */
                    RT_NULL,                        /* �߳��β� */
                   &rt_flag1_thread_stack[0],       /* �߳�ջ��ʼ��ַ */
                    sizeof(rt_flag1_thread_stack)); /* �߳�ջ��С����λΪ�ֽ� */
    
    /* ���̲߳��뵽�����б��� */
    rt_list_insert_before(&(rt_thread_priority_table[0]), &(rt_flag1_thread.tlist));
    
    /* ��ʼ���߳� */
    rt_thread_init(&rt_flag2_thread,                /* �߳̿��ƿ� */
                    flag2_thread_entry,             /* �߳���ڵ�ַ */
                    RT_NULL,                        /* �߳��β� */
                   &rt_flag2_thread_stack[0],       /* �߳�ջ��ʼ��ַ */
                    sizeof(rt_flag2_thread_stack)); /* �߳�ջ��С����λΪ�ֽ� */
    
    /* ���̲߳��뵽�����б��� */
    rt_list_insert_before(&(rt_thread_priority_table[1]), &(rt_flag2_thread.tlist));
    
    /* ����ϵͳ������ */
    rt_system_scheduler_start();
}

/* �����ʱ */
void delay (uint32_t count)
{
    while (count != 0) {
        count--;
    }
}

/* �߳�1 */
void flag1_thread_entry (void *p_arg)
{
    for (; ;){
#if 1
        flag1 = 1;
        delay(100);
        flag1 = 0;
        delay(100);
        
        /* �߳��л����������ֶ��л� */
        rt_schedule();
#else
        flag1 = 1;
        rt_thread_delay(2);
        flag1 = 0;
        rt_thread_delay(2);
#endif
    }
}

/* �߳�2 */
void flag2_thread_entry (void *p_arg)
{
    for (; ;){
#if 1
        flag2 = 1;
        delay(100);
        flag2 = 0;
        delay(100);
        
        /* �߳��л����������ֶ��л� */
        rt_schedule();
#else
        flag2 = 1;
        rt_thread_delay(2);
        flag2 = 0;
        rt_thread_delay(2);
#endif
    }
}


