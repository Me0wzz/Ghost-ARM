#include "ghost.h"

#define MAX_FILENAME_LEN 20

volatile unsigned int system_uptime = 0;

tcb_t initial_task;
tcb_t *current_task;
tcb_t *task_list[MAX_TASKS];
int task_cnt = 3;

volatile unsigned int *const TIMER0_INTCLR = (unsigned int *)0x101E200C;

void task_init(tcb_t *task, void (*func)(), int id, int prior) {
  task->pid = id;
  task->priority = prior;
  task->quantum = prior;
  task->state = STATE_READY;
  task->wake_at = 0;
  unsigned int *sp = task->stack + STACK_SIZE;
  task->context.pc = (unsigned int)func;
  task->context.sp = (unsigned int)&(task->stack[STACK_SIZE]);
  task->context.cpsr = 0x60000013; // SVC mode

  sp--;
  *sp = (unsigned int)func; // LR
  for (int i = 0; i < 13; i++) {
    sp--;
    *sp = 0;
  }
  task->context.sp = (unsigned int)sp;
}

int task_create(void (*entry)(), int priority) {
  disable_irq();
  int slot_idx = -1;
  for (int i = 0; i < MAX_TASKS; i++) {
    if (task_list[i] != NULL && task_list[i]->state == STATE_DEAD) {
      slot_idx = i;
      break;
    }
  }
  if (slot_idx == -1) {
    for (int i = 0; i < MAX_TASKS; i++) {
      if (task_list[i] == NULL) {
        slot_idx = i;
        break;
      }
    }
  }
  if (slot_idx == -1)
    return -1;
  tcb_t *new_task = (tcb_t *)malloc(sizeof(tcb_t));
  if (new_task == NULL)
    return -2;
  unsigned int *stack_top = &new_task->stack[STACK_SIZE];
  stack_top--;
  *stack_top = (unsigned int)entry;

  for (int i = 0; i < 13; i++) {
    stack_top--;
    *stack_top = 0;
  }
  new_task->context.sp = (unsigned int)stack_top;
  new_task->context.pc = (unsigned int)entry;
  new_task->context.cpsr = 0x13; // SVC
  new_task->context.lr = (unsigned int)entry;

  new_task->pid = task_cnt + 1;
  new_task->priority = priority;
  new_task->quantum = priority;
  new_task->state = STATE_READY;
  new_task->wake_at = 0;

  task_list[slot_idx] = new_task;
  if (slot_idx >= task_cnt)
    task_cnt = slot_idx + 1;
  enable_irq();
  return new_task->pid;
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
