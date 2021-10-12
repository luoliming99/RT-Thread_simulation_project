
; 导入其他文件的全局变量
    IMPORT rt_thread_switch_interrupt_flag
    IMPORT rt_interrupt_from_thread
    IMPORT rt_interrupt_to_thread


; 定义一些内核中的寄存器
SCB_VTOR        EQU     0xE000ED08  ; 向量表偏移寄存器
NVIC_INT_CTRL   EQU     0xE000ED04  ; 中断控制状态寄存器
NVIC_SYSPRI2    EQU     0xE000ED20  ; 系统优先级寄存器(2)
NVIC_PENDSV_PRI EQU     0x00FF0000  ; PendSV 优先级值(lowest)
NVIC_PENDSVSET  EQU     0x10000000  ; 触发 PendSV exception 的值

;-------------------------------------------------------------------------------
;              代码产生指令
;-------------------------------------------------------------------------------

    AREA |.text|, CODE, READONLY, ALIGN=2
    THUMB
    REQUIRE8
    PRESERVE8
        
;-------------------------------------------------------------------------------
; 函数原型：void rt_hw_context_switch_to (rt_uint32_t to);
; r0 --> to
; 该函数用于开启第一次线程切换
;-------------------------------------------------------------------------------

rt_hw_context_switch_to     PROC
    
    ; 导出 rt_hw_context_switch_to，让其具有全局属性，可以在C文件调用
    EXPORT rt_hw_context_switch_to
        
    ; 设置 rt_interrupt_to_thread 的值为下一个线程的栈指针SP的指针
    ; 将 rt_interrupt_to_thread 的地址加载到r1
    LDR     r1, =rt_interrupt_to_thread
    ; 将 r0 的值存储到 rt_interrupt_to_thread
    STR     r0, [r1]
    
    ; 设置 rt_interrupt_from_thread 的值为0，表示启动第一次线程切换
    ; 将 rt_interrupt_from_thread 的地址加载到r1
    LDR     r1, =rt_interrupt_from_thread
    ; 配置 r0 等于0
    MOV     r0, #0x0
    ; 将 r0 的值存储到 rt_interrupt_from_thread
    STR     r0, [r1]
    
    ; 设置中断标志位 rt_thread_switch_interrupt_flag 的值为1
    ; 将 rt_thread_switch_interrupt_flag 的地址加载到r1
    LDR     r1, =rt_thread_switch_interrupt_flag
    ; 配置 r0 等于1
    MOV     r0, #0x1
    ; 将 r0 的值存储到 rt_thread_switch_interrupt_flag
    STR     r0, [r1]
    
    ; 设置 PendSV 异常的优先级
    LDR     r0, =NVIC_SYSPRI2
    LDR     r1, =NVIC_PENDSV_PRI
    LDR.W   r2, [r0, #0x00]         ; 读
    ORR     r1, r1, r2              ; 改
    STR     r1, [r0]                ; 写
    
    ; 触发 PendSV 异常（产生上下文切换）
    LDR     r0, =NVIC_INT_CTRL
    LDR     r1, =NVIC_PENDSVSET
    STR     r1, [r0]
    
    ; 开中断
    CPSIE   F
    CPSIE   I
    
    ; 永远不会到达这里
    ENDP
    
;-------------------------------------------------------------------------------
; 函数原型：void PendSV_Handler (void);
; r0 --> switch from thread stack
; r1 --> switch to thread stack
; psr, pc, lr, r12, r3, r2, r1, r0 are pushed into [from] stack
;-------------------------------------------------------------------------------

PendSV_Handler PROC
    EXPORT PendSV_Handler
        
    ; 禁能中断，为了保护上下文切换不被中断
    MRS     r2, PRIMASK
    CPSID   I
    
    ; 获取中断标志位，看看是否为0
    ; 加载rt_thread_switch_interrupt_flag的地址到r0
    LDR     r0, =rt_thread_switch_interrupt_flag
    ; 加载rt_thread_switch_interrupt_flag的值到r1
    LDR     r1, [r0]
    ; 判断r1是否为0，为0则跳转到pendsv_exit
    CBZ     r1, pendsv_exit
    
    ; r1不为0则清0
    MOV     r1, #0x00
    ; 将r1的值存储到rt_thread_switch_interrupt_flag，即清0
    STR     r1, [r0]
    
    ; 判断rt_interrupt_from_thread的值是否为0
    ; 加载rt_interrupt_from_thread的地址到r0
    LDR     r0, =rt_interrupt_from_thread
    ; 加载rt_interrupt_from_thread的值到r1
    LDR     r1, [r0]
    ; 判断r1是否为0，为0则跳转到switch_to_thread
    ; 第一次线程切换时rt_interrupt_from_thread肯定为0，则跳转到switch_to_thread
    CBZ     r1, switch_to_thread
    
;--------------------------------- 上文保存 ------------------------------------
    ; 当进入PendSVC_Handler时，上一个线程运行的环境即：
    ; xPSR, PC(线程入口地址), R14, R12, R3, R2, R1, R0(线程的形参)
    ; 这些CPU寄存器的值会自动保存到线程的栈中，剩下的r4~r11需要手动保存
    ; 获取线程栈指针到r1
    MRS     r1, psp
    ; 将CPU寄存器r4~r11的值存储到r1指向的地址(每操作一次地址将递减一次)
    STMFD   r1!, {r4 - r11}
    ; 加载r0指向值到r0，即r0=rt_interrupt_from_thread
    LDR     r0, [r0]
    ; 将r1的值存储到r0，即更新线程栈sp
    STR     r1, [r0]

;--------------------------------- 下文切换 ------------------------------------
switch_to_thread
    ; 加载rt_interrupt_to_thread的地址到r1
    ; rt_interrupt_to_thread是一个全局变量，里面存的是线程栈指针SP的指针
    LDR     r1, =rt_interrupt_to_thread
    ; 加载rt_interrupt_to_thread的值到r1，即sp指针的指针
    LDR     r1, [r1]
    ; 加载rt_interrupt_to_thread的值到r1，即sp
    LDR     r1, [r1]
    ; 将线程栈指针r1(先操作后递增)指向的内容加载到CPU寄存器r4~r11
    LDMFD   r1!, {r4 - r11}
    ; 将线程栈指针更新到PSP
    MSR     psp, r1
    
pendsv_exit
    ; 恢复中断
    MSR     PRIMASK, r2
    
    ; 确保异常返回使用的栈指针是PSP，即LR寄存器的位2要为1
    ORR     lr, lr, #0x04
    ; 异常返回，这个时候栈中的剩下内容将会自动加载到CPU寄存器：
    ; xPSR, PC(线程入口地址), R14, R12, R3, R2, R1, R0(线程的形参)
    ; 同时PSP的值也将更新，即指向线程栈的栈顶
    BX      lr
    
    ; PendSV_Handler子程序结束
    ENDP
        
;-------------------------------------------------------------------------------
; 函数原型：void rt_hw_context_switch (rt_uint32_t from, rt_uint32_t to);
; r0 --> from
; r1 --> to
;-------------------------------------------------------------------------------
rt_hw_context_switch    PROC
    EXPORT  rt_hw_context_switch
        
    ; 设置中断标志位rt_thread_switch_interrupt_flag为1
    ; 加载rt_thread_switch_interrupt_flag的地址到r2
    LDR     r2, =rt_thread_switch_interrupt_flag
    ; 加载rt_thread_switch_interrupt_flag的值到r3
    LDR     r3, [r2]
    ; r3与1比较，相等则执行BEQ指令，否则不执行
    CMP     r3, #1
    BEQ     _reswitch
    ; 设置r3的值为1
    MOV     r3, #1
    ; 将r3的值存储到rt_thread_switch_interrupt_flag，即置1
    STR     r3, [r2]
    
    ; 设置rt_interrupt_from_thread的值
    ; 加载rt_interrupt_from_thread的地址到r2
    LDR     r2, =rt_interrupt_from_thread
    ; 存储r0的值到rt_interrupt_from_thread，即上一个线程栈指针(SP)的指针
    STR     r0, [r2]
    
_reswitch
    ; 设置rt_interrupt_to_thread的值
    ; 加载rt_interrupt_to_thread的地址到r2
    LDR     r2, =rt_interrupt_to_thread
    ; 存储r1的值到rt_interrupt_to_thread，即下一个线程栈指针(SP)的指针
    STR     r1, [r2]
    
    ; 触发PendSV异常，实现上下文切换
    LDR     r0, =NVIC_INT_CTRL
    LDR     r1, =NVIC_PENDSVSET
    STR     r1, [r0]
    ; 子程序返回
    BX      LR
    ; 子程序结束
    ENDP
        
;-------------------------------------------------------------------------------
; 函数原型：rt_base_t rt_hw_interrupt_disable (void);
; 关中断
;-------------------------------------------------------------------------------
rt_hw_interrupt_disable PROC
    EXPORT  rt_hw_interrupt_disable
    MRS     r0, PRIMASK
    CPSID   I
    BX      LR
    ENDP
     
;-------------------------------------------------------------------------------
; 函数原型：rt_base_t rt_hw_interrupt_enable (rt_base_t level);
; 开中断
;-------------------------------------------------------------------------------
rt_hw_interrupt_enable PROC
    EXPORT  rt_hw_interrupt_enable
    MSR     PRIMASK, r0
    BX      LR
    ENDP
        
        

    ; 当前文件指令代码要求4字节对齐
    ALIGN   4
    ; 汇编文件结束
    END