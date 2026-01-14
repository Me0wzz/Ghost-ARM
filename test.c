volatile unsigned int *const UART0DR = (unsigned int *)0x101f1000;
volatile unsigned int *const UART0FR = (unsigned int *)0x101f1018;

// SP804 Timer Initialization
volatile unsigned int *const TIMER0_BASE = (unsigned int *)0x101E2000;
volatile unsigned int *const TIMER0_LOAD =
    (unsigned int *)0x101E2000; // Offset 0x00
volatile unsigned int *const TIMER0_VALUE =
    (unsigned int *)0x101E2004; // Offset 0x04
volatile unsigned int *const TIMER0_CONTROL =
    (unsigned int *)0x101E2008; // Offset 0x08
volatile unsigned int *const TIMER0_INTCLR =
    (unsigned int *)0x101E200C; // Offset 0x0C

// Vector Interrupt Controller (VIC)
volatile unsigned int *const VIC_BASE = (unsigned int *)0x10140000;
volatile unsigned int *const VIC_INTENABLE =
    (unsigned int *)0x10140010; // Offset 0x10

volatile unsigned int system_uptime = 0;
#define TIMER_MAX_VAL 0xFFFFFFFF
#define LOW_PRIORITY 1
#define HIGH_PRIORITY 10

typedef struct {
  unsigned int r0;
  unsigned int r1;
  unsigned int r2;
  unsigned int r3;
  unsigned int r4;
  unsigned int r5;
  unsigned int r6;
  unsigned int r7;
  unsigned int r8;
  unsigned int r9;
  unsigned int r10;
  unsigned int r11;
  unsigned int r12;
  unsigned int sp;
  unsigned int lr;
  unsigned int pc;
  unsigned int cpsr;
} context_t;

typedef struct {
  context_t context;
  unsigned int pid;
  unsigned int stack[4096];
  unsigned int priority;
  unsigned int quantum;
} tcb_t;
tcb_t initial_task;
tcb_t task1;
tcb_t task2;
tcb_t shell_task;
tcb_t *current_task;

void task1_func() {
  while (1) {
    // safe_print("Task 1");
    for (volatile int i = 0; i < 1000000; i++)
      ;
  }
}

void task2_func() {
  while (1) {
    // safe_print("Task 2");
    for (volatile int i = 0; i < 1000000; i++)
      ;
  }
}

void shell_func() {
  char c;
  char cmd_buf[15];
  unsigned int buf_idx = 0;
  safe_print("Shell> ");
  while (1) {
    if ((*UART0FR & 0x10) == 0) {
      c = (char)(*UART0DR);
      if (c == '\r') {
        safe_print("\n");
        cmd_buf[buf_idx] = '\0';
        if (buf_idx == 0) {
        } else if (strcmp(cmd_buf, "help") == 0) {
          safe_print("Available commands:\n");
          safe_print("help - Show this message\n");
          safe_print("whoami - Show the current user\n");
          safe_print("clear - Clear the screen\n");
          safe_print("uptime - Show system uptime\n");
        } else if (strcmp(cmd_buf, "whoami") == 0) {
          safe_print("You are user 'ghost'\n");
        } else if (strcmp(cmd_buf, "clear") == 0) {
          safe_print("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
        } else if (strcmp(cmd_buf, "uptime") == 0) {
          safe_print("Uptime: ");
          safe_print_dec(system_uptime / 100);
          safe_print(" seconds\n");
        } else {
          safe_print("Unknown command: ");
          safe_print(cmd_buf);
          safe_print("\n");
        }
        buf_idx = 0;
        safe_print("Shell> ");
      } else if (c == 127 || c == 8) {
        if (buf_idx > 0) {
          buf_idx--;
          disable_irq();
          putc_uart0('\b');
          putc_uart0(' ');
          putc_uart0('\b');
          enable_irq();
        }
      } else {
        if (buf_idx < sizeof(cmd_buf) - 1) {
          cmd_buf[buf_idx] = c;
          disable_irq();
          buf_idx++;
          *UART0DR = (unsigned int)c; // Echo back
          enable_irq();
        }
      }
    }
  }
}

void task_init(tcb_t *task, void (*func)(), int id, int prior) {
  task->pid = id;
  task->priority = prior;
  task->quantum = prior;
  unsigned int *sp = task->stack + 4096;
  task->context.pc = (unsigned int)func;
  task->context.sp = (unsigned int)&(task->stack[4096]);
  task->context.cpsr = 0x60000013; // SVC mode
  sp--;
  *sp = (unsigned int)func;
  for (int i = 0; i < 13; i++) {
    sp--;
    *sp = 0;
  }

  // 2. 최종 스택 포인터(SP)를 TCB에 저장
  task->context.sp = (unsigned int)sp;
}

void init_multitasking() {
  task_init(&task1, task1_func, 1, LOW_PRIORITY);
  task_init(&task2, task2_func, 2, LOW_PRIORITY);
  task_init(&shell_task, shell_func, 0, HIGH_PRIORITY);
  current_task = &initial_task;
}

void schedule() {
  if (current_task != &initial_task) {
    if (current_task->quantum > 0) {
      current_task->quantum--;
      return;
    }
    current_task->quantum = current_task->priority;
  }
  if (current_task == &task1) {
    current_task = &task2;
  } else if (current_task == &task2) {
    current_task = &shell_task;
  } else {
    current_task = &task1;
  }
}

extern void enable_irq(void);
extern void disable_irq(void);

void c_irq_handler() {
  system_uptime++;

  *TIMER0_INTCLR = 1; // Clear the timer interrupt
}

void c_undefined_handler() {
  safe_print("Undefined Instruction Exception!\n");
  while (1)
    ;
}
void c_prefetch_handler() {
  safe_print("Prefetch Abort Exception!\n");
  while (1)
    ;
}
void c_data_handler() {
  safe_print("Data Abort Exception!\n");
  while (1)
    ;
}

void timer_init() {
  *TIMER0_CONTROL = 0;

  *TIMER0_LOAD = TIMER_MAX_VAL; // Load value
  // Bit 7: Enable interrupt
  // Bit 6: Periodic mode / Free-running mode
  // Bit 1: Prescale value (1 = 32-bit, 0 = 16-bit)
  // 1000 0010 = 0x82
  *TIMER0_CONTROL = 0x82;
}

void interrupt_init() {
  *TIMER0_LOAD = 10000;
  *TIMER0_CONTROL = 0xE2;
  *VIC_INTENABLE = 1 << 4; // Enable Timer0 interrupt (bit 4)
}

unsigned int uptime() {
  unsigned int current_value = *TIMER0_VALUE;
  unsigned int elapsed = TIMER_MAX_VAL - current_value;
  return elapsed / 1000000;
}

void print_uart0(const char *s) {
  while (*s != '\0') {
    *UART0DR = (unsigned int)(*s);
    s++;
  }
}

void putc_uart0(char c) { *UART0DR = (unsigned int)c; }

void print_dec(unsigned int num) {
  if (num == 0) {
    putc_uart0('0');
    return;
  }
  char buf[10];
  int i = 0;
  while (num > 0) {
    buf[i++] = (char)((num % 10) + '0');
    num /= 10;
  }
  for (int j = i - 1; j >= 0; j--) {
    putc_uart0(buf[j]);
  }
}

void safe_print(const char *s) {
  disable_irq();
  while (*s != '\0') {
    *UART0DR = (unsigned int)(*s++);
  }
  enable_irq();
}

void safe_print_dec(unsigned int num) {
  disable_irq();
  if (num == 0) {
    *UART0DR = (unsigned int)'0';
    enable_irq();
    return;
  }
  char buf[16];
  int i = 0;
  while (num > 0) {
    buf[i++] = (num % 10) + '0';
    num /= 10;
  }
  for (int j = i - 1; j >= 0; j--)
    *UART0DR = (unsigned int)buf[j];
  enable_irq();
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 != '\0' && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return *s1 - *s2;
}

void c_entry() {
  interrupt_init();
  init_multitasking();

  enable_irq();
  while (1)
    ;
}
