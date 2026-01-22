#include "ghost.h"

#define VIC_INTENABLE ((volatile unsigned int *)0x10140010)

volatile unsigned int *const TIMER0_LOAD = (unsigned int *)0x101E2000;
volatile unsigned int *const TIMER0_CONTROL = (unsigned int *)0x101E2008;

tcb_t task1, task2, shell_task;

void task1_func() {
  while (1) {
    // safe_print("Task 1\n");
    for (volatile int i = 0; i < 1000000; i++)
      ;
  }
}

void task2_func() {
  while (1) {
    // safe_print("Task 2\n");
    for (volatile int i = 0; i < 1000000; i++)
      ;
  }
}

void hw_init() {
  *TIMER0_LOAD = 10000; // 10ms
  *TIMER0_CONTROL = 0xE2;
  *VIC_INTENABLE = 1 << 4;
}

void c_entry() {
  hw_init();
  mm_init();
  task_init(&task1, task1_func, 1, LOW_PRIORITY);
  task_init(&task2, task2_func, 2, LOW_PRIORITY);
  task_init(&shell_task, shell_func, 0, HIGH_PRIORITY);

  task_list[0] = &task1;
  task_list[1] = &task2;
  task_list[2] = &shell_task;

  current_task = &initial_task;
  enable_irq();
  while (1)
    ;
}