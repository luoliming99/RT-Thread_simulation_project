#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__
#include "rtconfig.h"
#include "rtthread.h"

/* 已知一个结构体里面的成员的地址，反推出该结构体的首地址 */
#define rt_container_of(ptr, type, member) \
    ((type *)((char *)ptr - (unsigned long)(&((type *)0)->member)))

#define rt_list_entry(node, type, member) \
    rt_container_of(node, type, member)

extern rt_list_t rt_thread_priority_table[RT_THREAD_PRIORITY_MAX];

extern struct rt_thread *rt_current_thread;
    
extern rt_uint32_t rt_thread_ready_priority_group;

void rt_system_scheduler_init (void);

void rt_system_scheduler_start (void);

void rt_schedule (void);
    
void rt_schedule_insert_thread (struct rt_thread *thread);
    
void rt_schedule_remove_thread (struct rt_thread *thread);

#endif

