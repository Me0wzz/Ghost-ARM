#ifndef GHOST_H
#define GHOST_H

#define UART0_BASE 0x101f1000
#define TIMER0_BASE 0x101E2000
#define VIC_BASE 0x10140000

#define UART0DR ((volatile unsigned int *)0x101f1000)
#define UART0FR ((volatile unsigned int *)0x101f1018)

#define TIMER_MAX_VAL 0xFFFFFFFF
#define LOW_PRIORITY 1
#define HIGH_PRIORITY 10

#define STATE_DEAD 0
#define STATE_READY 1
#define STATE_WAIT 2

typedef struct {
  unsigned int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12;
  unsigned int sp, lr, pc, cpsr;
} context_t;

typedef struct {
  context_t context;
  unsigned int pid;
  unsigned int stack[4096];
  unsigned int priority;
  unsigned int quantum;
  unsigned int state;
  unsigned int wake_at;
} tcb_t;

extern volatile unsigned int system_uptime;
extern tcb_t *task_list[3];
extern int task_cnt;
extern tcb_t *current_task;
extern tcb_t initial_task;

// lib.c
unsigned int atoi(const char *str);
int strcmp(const char *s1, const char *s2);
void safe_print(const char *s);
void safe_print_dec(unsigned int num);
void putc_uart0(char c);

// kernel.c
void init_multitasking();
void schedule();
void sleep(unsigned int seconds);
void task_init(tcb_t *task, void (*func)(), int id, int prior);

// shell.c
void shell_func();

// ASM functions
extern void enable_irq(void);
extern void disable_irq(void);

#endif