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

rt_err_t rt_thread_init (struct rt_thread *thread,
                         void (*entry) (void *parameter),
                         void             *parameter,
                         void             *stack_start,
                         rt_uint32_t       stack_size);
    
void rt_thread_delay (rt_tick_t tick);

char *rt_strncpy (char *dst, const char *src, rt_ubase_t n);

void rt_object_init(struct  rt_object            *object,
                    enum    rt_object_class_type  type,
                    const   char                 *name);
                    
void rt_interrupt_enter (void);
                    
void rt_interrupt_leave (void);
                    
void rt_thread_idle_init (void);
                    
rt_err_t rt_thread_startup (rt_thread_t thread);
                    
#endif

