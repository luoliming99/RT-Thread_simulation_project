#ifndef __RTTHREAD_H__
#define __RTTHREAD_H__
#include "rtdef.h"

rt_inline void rt_list_init (rt_list_t *l)
{
    l->next = l->prev = l;
}

/* 在双向链表表头后面插入一个节点 */
rt_inline void rt_list_insert_after (rt_list_t *l, rt_list_t *n)
{
    l->next->prev = n;
    n->next = l->next;
    n->prev = l;
    l->next = n;
}

/* 在双向链表表头前面插入一个节点 */
rt_inline void rt_list_insert_before (rt_list_t *l, rt_list_t *n)
{
    l->prev->next = n;
    n->prev = l->prev;
    n->next = l;
    l->prev = n;
}

/* 从双向链表删除一个节点 */
rt_inline void rt_list_remove (rt_list_t *n)
{
    n->prev->next = n->next;
    n->next->prev = n->prev;
    n->next = n->prev = n;
}

rt_inline rt_bool_t rt_list_isempty (rt_list_t *l)
{
    if (l->next == l && l->prev == l) {
        return RT_TRUE;
    }
    return RT_FALSE;
}

/* 已知一个结构体里面的成员的地址，反推出该结构体的首地址 */
#define rt_container_of(ptr, type, member) \
    ((type *)((char *)ptr - (unsigned long)(&((type *)0)->member)))

#define rt_list_entry(node, type, member) \
    rt_container_of(node, type, member)

int __rt_ffs (int value);

char *rt_strncpy (char *dst, const char *src, rt_ubase_t n);

void rt_object_init(struct  rt_object            *object,
                    enum    rt_object_class_type  type,
                    const   char                 *name);
                    
void rt_interrupt_enter (void);
                    
void rt_interrupt_leave (void);
                    
void rt_thread_idle_init (void);
                    
/**< 定时器相关函数 */                
rt_tick_t rt_tick_get (void);    
void rt_syster_timer_init (void);
void rt_timer_init (rt_timer_t  timer,
                    const char *name,
                    void (*timeout) (void *parameter),
                    void       *parameter,
                    rt_tick_t   time,
                    rt_uint8_t  flag);
rt_err_t rt_timer_stop (rt_timer_t timer);
rt_err_t rt_timer_control (rt_timer_t timer, int cmd, void *arg);
rt_err_t rt_timer_start (rt_timer_t timer);
void rt_timer_check (void);
void rt_thread_timeout (void *parameter);
rt_err_t rt_thread_sleep (rt_tick_t tick);                    

/**< 线程相关函数 */
rt_thread_t rt_thread_self (void);
rt_err_t rt_thread_init (struct rt_thread *thread,
                         const char       *name,
                         void (*entry) (void *parameter),
                         void             *parameter,
                         void             *stack_start,
                         rt_uint32_t       stack_size,
                         rt_uint8_t        priority,
                         rt_uint32_t       tick);
rt_err_t rt_thread_delay (rt_tick_t tick);
rt_err_t rt_thread_suspend(rt_thread_t thread);
rt_err_t rt_thread_resume (rt_thread_t thread);
rt_err_t rt_thread_startup (rt_thread_t thread);
rt_err_t rt_thread_yield (void);                         
                         
#endif

