volatile unsigned int *const UART0DR = (unsigned int *)0x101f1000;
volatile unsigned int *const UART0FR = (unsigned int *)0x101f1018;
void print_uart0(const char *s) {
  while (*s != '\0') {
    *UART0DR = (unsigned int)(*s);
    s++;
  }
}

void putc_uart0(char c) { *UART0DR = (unsigned int)c; }

int strcmp(const char *s1, const char *s2) {
  while (*s1 != '\0' && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return *s1 - *s2;
}

void c_entry() {
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
        } else if (strcmp(cmd_buf, "whoami") == 0) {
          print_uart0("You are user 'ghost'\n");
        } else if (strcmp(cmd_buf, "clear") == 0) {
          print_uart0("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
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
