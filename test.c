volatile unsigned int *const UART0DR = (unsigned int *)0x101f1000;

void c_entry() {
  const char *message = "Hello, ARM World!\n";
  while (*message != '\0') {
    *UART0DR = (unsigned int)(*message);
    message++;
  }
  while (1)
    ;
}