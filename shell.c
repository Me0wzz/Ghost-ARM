#include "ghost.h"

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
          safe_print("jobs - List running tasks\n");
          safe_print("kill <pid> - Kill a task by PID\n");
          safe_print("stop <pid> - Pause a task by PID\n");
          safe_print("resume <pid> - Resume a paused task by PID\n");
        } else if (strcmp(cmd_buf, "whoami") == 0) {
          safe_print("You are user 'ghost'\n");
        } else if (strcmp(cmd_buf, "clear") == 0) {
          safe_print("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
        } else if (strcmp(cmd_buf, "uptime") == 0) {
          safe_print("Uptime: ");
          safe_print_dec(system_uptime / 100);
          safe_print(" seconds\n");
        } else if (strcmp(cmd_buf, "jobs") == 0) {
          safe_print("Running tasks:\n");
          for (int i = 0; i < task_cnt; i++) {
            tcb_t *task = task_list[i];
            safe_print("PID: ");
            safe_print_dec(task->pid);
            safe_print("    ");
            safe_print("Priority: ");
            safe_print_dec(task->priority);

            safe_print("    ");
            safe_print("State: ");
            if (task->state == STATE_READY)
              safe_print("READY");
            else if (task->state == STATE_DEAD)
              safe_print("DEAD");
            else if (task->state == STATE_WAIT)
              safe_print("WAIT");
            else
              safe_print("UNKNOWN");
            safe_print("\n");
          }

        } else if (cmd_buf[0] == 'k' && cmd_buf[1] == 'i' &&
                   cmd_buf[2] == 'l' && cmd_buf[3] == 'l') {
          int target_pid = atoi(&cmd_buf[5]);
          int found = 0;

          for (int i = 0; i < task_cnt; i++) {
            if (task_list[i]->pid == target_pid) {
              if (target_pid == 0) {
                safe_print("Cannot kill shell \n");
              } else {
                task_list[i]->state = STATE_DEAD;
                safe_print("Killed process with PID: ");
                safe_print_dec(target_pid);
                safe_print("\n");
              }
              found = 1;
              break;
            }
          }
          if (!found) {
            safe_print("No such process with PID: ");
            safe_print_dec(target_pid);
            safe_print("\n");
          }
        } else if (cmd_buf[0] == 's' && cmd_buf[1] == 't' &&
                   cmd_buf[2] == 'o' && cmd_buf[3] == 'p') {
          int target_pid = atoi(&cmd_buf[5]);
          int found = 0;

          for (int i = 0; i < task_cnt; i++) {
            if (task_list[i]->pid == target_pid) {
              if (target_pid == 0) {
                safe_print("Cannot stop shell \n");
              } else if (task_list[i]->state == STATE_DEAD) {
                safe_print("Cannot pause a dead process with PID: ");
                safe_print_dec(target_pid);
                safe_print("\n");
              } else if (task_list[i]->state == STATE_WAIT) {
                safe_print("Process with PID: ");
                safe_print_dec(target_pid);
                safe_print(" is already paused\n");
              } else {
                task_list[i]->state = STATE_WAIT;
                safe_print("Paused process with PID: ");
                safe_print_dec(target_pid);
                safe_print("\n");
              }
              found = 1;
              break;
            }
          }
          if (!found) {
            safe_print("No such process with PID: ");
            safe_print_dec(target_pid);
            safe_print("\n");
          }
        } else if (cmd_buf[0] == 'r' && cmd_buf[1] == 'e' &&
                   cmd_buf[2] == 's' && cmd_buf[3] == 'u' &&
                   cmd_buf[4] == 'm' && cmd_buf[5] == 'e') {
          int target_pid = atoi(&cmd_buf[7]);
          int found = 0;

          for (int i = 0; i < task_cnt; i++) {
            if (task_list[i]->pid == target_pid) {
              if (task_list[i]->state == STATE_DEAD) {
                safe_print("Cannot resume a dead process with PID: ");
                safe_print_dec(target_pid);
                safe_print("\n");
              } else if (task_list[i]->state == STATE_READY) {
                safe_print("Process with PID: ");
                safe_print_dec(target_pid);
                safe_print(" is already running\n");
              } else {
                task_list[i]->state = STATE_READY;
                safe_print("Resumed process with PID: ");
                safe_print_dec(target_pid);
                safe_print("\n");
              }
              found = 1;
              break;
            }
          }
          if (!found) {
            safe_print("No such process with PID: ");
            safe_print_dec(target_pid);
            safe_print("\n");
          }
        } else if (cmd_buf[0] == 's' && cmd_buf[1] == 'l' &&
                   cmd_buf[2] == 'e' && cmd_buf[3] == 'e' &&
                   cmd_buf[4] == 'p') {
          int sec = atoi(&cmd_buf[6]);
          safe_print("Sleeping for ");
          safe_print_dec(sec);
          safe_print(" seconds...\n");
          sleep(sec);
          safe_print("Awake!\n");

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
