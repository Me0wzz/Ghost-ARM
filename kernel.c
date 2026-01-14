#include "ghost.h"

volatile unsigned int system_uptime = 0;

tcb_t initial_task;
tcb_t *current_task;
tcb_t *task_list[3];
int task_cnt = 3;

volatile unsigned int *const TIMER0_INTCLR = (unsigned int *)0x101E200C;

void task_init(tcb_t *task, void (*func)(), int id, int prior) {
  task->pid = id;
  task->priority = prior;
  task->quantum = prior;
  task->state = STATE_READY;
  task->wake_at = 0;
  unsigned int *sp = task->stack + 4096;
  task->context.pc = (unsigned int)func;
  task->context.sp = (unsigned int)&(task->stack[4096]);
  task->context.cpsr = 0x60000013; // SVC mode

  sp--;
  *sp = (unsigned int)func; // LR
  for (int i = 0; i < 13; i++) {
    sp--;
    *sp = 0;
  }
  task->context.sp = (unsigned int)sp;
}

void sleep(unsigned int seconds) {
  tcb_t *t = current_task;
  unsigned int tick = seconds * 100; // 1 tick = 10ms
  t->wake_at = system_uptime + tick;
  t->state = STATE_WAIT;
  t->quantum = 0;
  while (t->state == STATE_WAIT)
    ;
}

void schedule() {
  for (int i = 0; i < task_cnt; i++) {
    if (task_list[i]->state == STATE_WAIT && task_list[i]->wake_at > 0) {
      if (system_uptime >= task_list[i]->wake_at) {
        task_list[i]->state = STATE_READY;
        task_list[i]->wake_at = 0;
      }
    }
  }
  if (current_task != &initial_task) {
    if (current_task->quantum > 0 && current_task->state == STATE_READY) {
      current_task->quantum--;
      return;
    }
    current_task->quantum = current_task->priority;
  }

  int next_idx = -1;
  int current_idx = 0;
  for (int i = 0; i < task_cnt; i++) {
    if (task_list[i] == current_task) {
      current_idx = i;
      break;
    }
  }

  for (int i = 1; i <= task_cnt; i++) {
    int idx = current_idx + i;
    if (idx >= task_cnt)
      idx -= task_cnt;

    if (task_list[idx]->state == STATE_READY) {
      next_idx = idx;
      break;
    }
  }

  if (next_idx != -1)
    current_task = task_list[next_idx];
}

void c_irq_handler() {
  system_uptime++;
  *TIMER0_INTCLR = 1;
}

void c_undefined_handler() {
  safe_print("Undefined Inst!\n");
  while (1)
    ;
}
void c_prefetch_handler() {
  safe_print("Prefetch Abort!\n");
  while (1)
    ;
}
void c_data_handler() {
  safe_print("Data Abort!\n");
  while (1)
    ;
}