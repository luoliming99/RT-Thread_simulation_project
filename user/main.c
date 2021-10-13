#include "rtdef.h"
#include "rtconfig.h"
#include "rtthread.h"
#include "scheduler.h"
#include "rthw.h"
#include "ARMCM7.h"

rt_uint8_t flag1;
rt_uint8_t flag2;

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
    rt_hw_interrupt_disable();
    
    SysTick_Config(SystemCoreClock / RT_TICK_PER_SECOND);

    /* ��������ʼ�� */
    rt_system_scheduler_init();
    
    /* ��ʼ�������߳� */
    rt_thread_idle_init();
       
    
    /* ��ʼ���߳� */
    rt_thread_init(&rt_flag1_thread,                /* �߳̿��ƿ� */
                    "rt_flag1_thread",              /* �߳����֣��ַ�����ʽ */
                    flag1_thread_entry,             /* �߳���ڵ�ַ */
                    RT_NULL,                        /* �߳��β� */
                   &rt_flag1_thread_stack[0],       /* �߳�ջ��ʼ��ַ */
                    sizeof(rt_flag1_thread_stack),  /* �߳�ջ��С����λΪ�ֽ� */
                    2);                             /* ���ȼ� */
    
    /* ���̲߳��뵽�����б��� */
    rt_thread_startup(&rt_flag1_thread);
    
    /* ��ʼ���߳� */
    rt_thread_init(&rt_flag2_thread,                /* �߳̿��ƿ� */
                    "rt_flag2_thread",              /* �߳����֣��ַ�����ʽ */
                    flag2_thread_entry,             /* �߳���ڵ�ַ */
                    RT_NULL,                        /* �߳��β� */
                   &rt_flag2_thread_stack[0],       /* �߳�ջ��ʼ��ַ */
                    sizeof(rt_flag2_thread_stack),  /* �߳�ջ��С����λΪ�ֽ� */
                    3);                             /* ���ȼ� */
    
    /* ���̲߳��뵽�����б��� */
    rt_thread_startup(&rt_flag2_thread);
    
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
        flag1 = 1;
        rt_thread_delay(2);
        flag1 = 0;
        rt_thread_delay(2);
    }
}

/* �߳�2 */
void flag2_thread_entry (void *p_arg)
{
    for (; ;){
        flag2 = 1;
        rt_thread_delay(2);
        flag2 = 0;
        rt_thread_delay(2);
    }
}


