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

#define TIMER_MAX_VAL 0xFFFFFFFF

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
  unsigned int stack[1024];
} tcb_t;
tcb_t initial_task;
tcb_t task1;
tcb_t task2;
tcb_t *current_task;

void task1_func() {
  while (1) {
    print_uart0("Task 1");
    for (volatile int i = 0; i < 1000000; i++)
      ;
  }
}

void task2_func() {
  while (1) {
    print_uart0("Task 2");
    for (volatile int i = 0; i < 1000000; i++)
      ;
  }
}

void task_init(tcb_t *task, void (*func)(), int id) {
  task->pid = id;
  unsigned int *sp = task->stack + 1024;
  task->context.pc = (unsigned int)func;
  task->context.sp = (unsigned int)&(task->stack[1024]);
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
  task_init(&task1, task1_func, 1);
  task_init(&task2, task2_func, 2);
}

void schedule() {
  if (current_task == &task1) {
    current_task = &task2;
  } else {
    current_task = &task1;
  }
}

volatile unsigned int system_uptime = 0;

extern void enable_irq(void);
void c_irq_handler() {
  system_uptime++;

  *TIMER0_INTCLR = 1; // Clear the timer interrupt
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
  *TIMER0_LOAD = 1000000;
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

int strcmp(const char *s1, const char *s2) {
  while (*s1 != '\0' && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return *s1 - *s2;
}

void c_entry() {
  interrupt_init();
  enable_irq();
  init_multitasking();
  char c;
  char cmd_buf[15];
  unsigned int buf_idx = 0;
  print_uart0("Shell> ");
  while (1) {
    if ((*UART0FR & 0x10) == 0) {
      c = (char)(*UART0DR);
      if (c == '\r') {
        print_uart0("\n");
        cmd_buf[buf_idx] = '\0';
        if (buf_idx == 0) {
        } else if (strcmp(cmd_buf, "help") == 0) {
          print_uart0("Available commands:\n");
          print_uart0("help - Show this message\n");
          print_uart0("whoami - Show the current user\n");
          print_uart0("clear - Clear the screen\n");
          print_uart0("uptime - Show system uptime\n");
        } else if (strcmp(cmd_buf, "whoami") == 0) {
          print_uart0("You are user 'ghost'\n");
        } else if (strcmp(cmd_buf, "clear") == 0) {
          print_uart0("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
        } else if (strcmp(cmd_buf, "uptime") == 0) {
          print_uart0("Uptime: ");
          print_dec(system_uptime);
          print_uart0(" seconds\n");
        } else {
          print_uart0("Unknown command: ");
          print_uart0(cmd_buf);
          print_uart0("\n");
        }
        buf_idx = 0;
        print_uart0("Shell> ");
      } else if (c == 127 || c == 8) {
        if (buf_idx > 0) {
          buf_idx--;
          putc_uart0('\b');
          putc_uart0(' ');
          putc_uart0('\b');
        }
      } else {
        if (buf_idx < sizeof(cmd_buf) - 1) {
          cmd_buf[buf_idx] = c;
          buf_idx++;
          *UART0DR = (unsigned int)c; // Echo back
        }
      }
    }
  }
}
