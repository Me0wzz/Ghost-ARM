#include "ghost.h"

/*
int atoi(const char *str) {
  int result = 0;
  int sign = 1;
  if (*str == '-') {
    sign = -1;
    str++;
  }
  while (*str >= '0' && *str <= '9') {
    result = result * 10 + (*str - '0');
    str++;
  }
  return sign * result;
}
*/

unsigned int atoi(const char *str) {
  unsigned int result = 0;
  while (*str >= '0' && *str <= '9') {
    result = result * 10 + (*str - '0');
    str++;
  }
  return result;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 != '\0' && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return *s1 - *s2;
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
