/**
  *****************************************************************************************
  *
  * @name    port.c
  * @brief   os shedule
  * @author  yuzr
  * @time    2019/5/21
  *
  *****************************************************************************************
  */

#include "ARMCM3.h"

//
// Define PendSV priority value (lowest).
//
#define NVIC_PENDSV_PRI  0xFF

//
// Value to trigger PendSV exception.
//
#define NVIC_ICSR_PENDSVSET  0x10000000

//
// Value to trigger PendSV exception.
//
#define NVIC_ICSR   (*((uint32_t *)0xE000ED04))

//
// System priority register for core (priority 14).
//
#define NVIC_SYSPRI14  (*((uint8_t *)0xE000ED22))

//
// Task control block
//
typedef struct os_tcb
{
  uint32_t *stack_addr;
}os_tcb,*os_tcb_p;

//
// Define the global task control block.
//
/* Current task control block */
os_tcb_p os_current_tcb;
/* High priority task control block in ready mode */
os_tcb_p os_high_ready_tcb;
/* Idel task control block */
os_tcb_p os_idel_tcb;

//
// Define the expected stack size.
//
#define OS_EXPECT_STACK_SIZE  256
uint32_t os_expect_stack[OS_EXPECT_STACK_SIZE];
#define OS_EXPECT_STACK_BASE  (os_expect_stack + OS_EXPECT_STACK_SIZE -1)

/**
  *
  * @brief   os_shedule_trigger,Trigger PendSVC handler.
  * @param   none
  * @retval  none
  *
  */
void os_shedule_trigger(void)
{
  NVIC_ICSR = NVIC_ICSR_PENDSVSET;
  return ;
}

/**
  *
  * @brief   os_start,Start OS.
  * @param   none
  * @retval  none
  *
  */
void os_start(void)
{
  //
  // Set the PendSV exception priority
  //
  NVIC_SYSPRI14 = NVIC_PENDSV_PRI;
  
  //
  // Initialize the PSP and MSP register.
  //
  __set_PSP(0);
  __set_MSP((uint32_t)OS_EXPECT_STACK_BASE);

  //
  // Trigger the PendSV exception (causes context switch).
  //
  NVIC_ICSR = NVIC_ICSR_PENDSVSET;
  
  //
  // Enable interrupts at processor level
  //
  __asm {
    CPSIE   I
  }
  
  return ;
}

/**
  *
  * @brief   PendSV_Handler,pendsv interrupt service handler.
  * @param   none
  * @retval  none
  * @note    1.The PSP register will be 0 in the first PendSV_Handler occurr.Then make the
  *           os_current_tcb = os_high_ready_tcb,
  *         
  *          2.We just pop the context from current process stack.
  */
__asm void PendSV_Handler(void)
{
  extern  os_current_tcb
  extern  os_high_ready_tcb

  CPSID   I                                        ; Prevent interruption during context switch
  MRS     R0, PSP                                  ; PSP is process stack pointer
  CBZ     R0, os_first_shedule                     ; Skip register save the first time
   
  SUBS    R0, R0, #0x20                            ; Save remaining regs r4-11 on process stack
  STM     R0, {R4-R11}

  LDR     R1, =os_current_tcb                      ; OSTCBCur->OSTCBStkPtr = SP;
  LDR     R1, [R1]
  STR     R0, [R1]                                 ; R0 is SP of process being switched out

; At this point, entire context of process has been saved

os_first_shedule
  LDR     R0, =os_current_tcb                      ; OSTCBCur  = OSTCBHighRdy;
  LDR     R1, =os_high_ready_tcb
  LDR     R2, [R1]
  STR     R2, [R0]

  LDR     R0, [R2]                                 ; R0 is new process SP; SP = OSTCBHighRdy->OSTCBStkPtr;
  
  LDM     R0, {R4-R11}                             ; Restore r4-11 from new process stack
  ADDS    R0, R0, #0x20
            
  MSR     PSP, R0                                  ; Load PSP with new process SP
  ORR     LR, LR, #0x04                            ; Ensure exception return uses process stack
    
  CPSIE   I
  BX      LR                                       ; Exception return will restore remaining context
}
