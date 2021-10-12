#ifndef __CPUPORT_H__
#define __CPUPORT_H__
#include "rtdef.h"

rt_uint8_t *rt_hw_stack_init (void       *tentry,
                              void       *parameter,
                              rt_uint8_t *stack_addr);

#endif
