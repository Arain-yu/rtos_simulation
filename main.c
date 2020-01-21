/**
  *****************************************************************************************
  *
  * @name    main.c
  * @author  yuzrain
  *
  *****************************************************************************************
  */

#include "stdio.h"
#include "ARMCM3.h"

#define TASK_1_STK_SIZE 1024
#define TASK_2_STK_SIZE 1024

typedef void (*os_task_handler_t)(void);

//------------------------------------------------------------------------------
// Task control block
//------------------------------------------------------------------------------
typedef struct os_tcb
{
  uint32_t *stack_addr;
}os_tcb,*os_tcb_p;

//------------------------------------------------------------------------------
// Hold task 1/2 task control block
//------------------------------------------------------------------------------
static os_tcb task1_tcb;
static os_tcb task2_tcb;
static uint32_t task1_stack[TASK_1_STK_SIZE];
static uint32_t task2_stack[TASK_2_STK_SIZE];

//------------------------------------------------------------------------------
// Hold task 1/2 task shedule
//------------------------------------------------------------------------------
extern os_tcb_p os_current_tcb; 
extern os_tcb_p os_high_ready_tcb;
extern void os_start(void);
extern void os_shedule_trigger(void);

/**
 * @brief switch task [task1->task2->task1...]
 *
 */
void Task_Switch()
{
  if(os_current_tcb == &task1_tcb)
    os_high_ready_tcb=&task2_tcb;
  else
    os_high_ready_tcb=&task1_tcb;
 
  os_shedule_trigger();
}

/**
 * @brief task1
 *
 */
void task_1()
{
    uint32_t i;

    while (1)
    {
        i = 1000;
        while (i--);
        Task_Switch();
        i = 1000;
        while (i--);
        Task_Switch();
    }
}

/**
 * @brief task2
 *
 */
void task_2()
{
    uint32_t i;
  
    while (1)
    {
        i = 1000;
        while (i--);
        Task_Switch();
        i = 1000;
        while (i--);
        Task_Switch();
    }
}

void Task_End(void)
{
    /* We can not reach here */
    while(1);
}

/**
 * @brief Create task stack
 *
 */
void Task_Create(os_tcb *tcb, os_task_handler_t task, uint32_t *stk)
{
    uint32_t  *p_stk;
    p_stk      = stk;
    p_stk      = (uint32_t *)((uint32_t)(p_stk) & 0xFFFFFFF8u);
    
    *(--p_stk) = (uint32_t)0x01000000uL;                          //xPSR
    *(--p_stk) = (uint32_t)task;                                  // Entry Point
    *(--p_stk) = (uint32_t)Task_End;                                     // R14 (LR)
    *(--p_stk) = (uint32_t)0x12121212uL;                          // R12
    *(--p_stk) = (uint32_t)0x03030303uL;                          // R3
    *(--p_stk) = (uint32_t)0x02020202uL;                          // R2
    *(--p_stk) = (uint32_t)0x01010101uL;                          // R1
    *(--p_stk) = (uint32_t)0x00000000u;                           // R0
    
    *(--p_stk) = (uint32_t)0x11111111uL;                          // R11
    *(--p_stk) = (uint32_t)0x10101010uL;                          // R10
    *(--p_stk) = (uint32_t)0x09090909uL;                          // R9
    *(--p_stk) = (uint32_t)0x08080808uL;                          // R8
    *(--p_stk) = (uint32_t)0x07070707uL;                          // R7
    *(--p_stk) = (uint32_t)0x06060606uL;                          // R6
    *(--p_stk) = (uint32_t)0x05050505uL;                          // R5
    *(--p_stk) = (uint32_t)0x04040404uL;                          // R4
    
    tcb->stack_addr=p_stk;
}

/**
 * @brief Test
 *
 */
int main()
{ 
  Task_Create(&task1_tcb,task_1,&task1_stack[TASK_1_STK_SIZE-1]);
  Task_Create(&task2_tcb,task_2,&task2_stack[TASK_1_STK_SIZE-1]);
    
  os_high_ready_tcb=&task1_tcb;
  
  os_start();
  
  while(1)
  {}
}
