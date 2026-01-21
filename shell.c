#include "flash.h"
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
          safe_print("memtest - Test dynamic memory allocation\n");
          safe_print("gui - Enter GUI mode\n");
          safe_print("jobs - List running tasks\n");
          safe_print("kill <pid> - Kill a task by PID\n");
          safe_print("stop <pid> - Pause a task by PID\n");
          safe_print("resume <pid> - Resume a paused task by PID\n");
          safe_print(
              "sleep <seconds> - Sleep the shell for specified seconds\n");
          safe_print("tetris - Start TETRIS game\n");
          safe_print("format - Format the flash storage\n");
          safe_print("ls - List files in flash storage\n");
          safe_print("save <name> <text> - Save text to a file\n");
          safe_print("load <name> - Load and display a file's content\n");
        } else if (strcmp(cmd_buf, "whoami") == 0) {
          safe_print("You are user 'ghost'\n");
        } else if (strcmp(cmd_buf, "clear") == 0) {
          clrscr();
        } else if (strcmp(cmd_buf, "ls") == 0) {
          fs_ls();
        } else if (strcmp(cmd_buf, "format") == 0) {
          fs_format();
          safe_print("Disk formatted.\n");
        } else if (strcmp(cmd_buf, "score") == 0) {
          int val = fs_load_score();
          safe_print("Your TETRIS score: ");
          safe_print_dec(val);
          safe_print("\n");

        } else if (cmd_buf[0] == 's' && cmd_buf[1] == 'a' &&
                   cmd_buf[2] == 'v' && cmd_buf[3] == 'e') {
          char name[16];
          char content[32];

          int i = 5;
          int j = 0;
          while (cmd_buf[i] != ' ' && cmd_buf[i] != 0 && j < 15) {
            name[j++] = cmd_buf[i++];
          }
          name[j] = 0;
          if (cmd_buf[i] == ' ') {
            i++;
            int k = 0;
            while (cmd_buf[i] != 0 && k < 31) {
              content[k++] = cmd_buf[i++];
            }
            content[k] = 0;
            fs_save_file(name, content, k + 1);

          } else {
            safe_print("Usage: save <name> <text>\n");
          }

        } else if (cmd_buf[0] == 'l' && cmd_buf[1] == 'o' &&
                   cmd_buf[2] == 'a' && cmd_buf[3] == 'd') {
          char name[16];
          int i = 5;
          int j = 0;
          while (cmd_buf[i] != 0 && j < 15)
            name[j++] = cmd_buf[i++];
          name[j] = 0;
          char buf[100];
          int len = fs_load_file(name, buf, 100);
          if (len != -1) {
            safe_print("Content: [");
            safe_print(name);
            safe_print(": ");
            safe_print(buf);
            safe_print("]\n");
          } else {
            safe_print("File not found.\n");
          }
        }

        else if (strcmp(cmd_buf, "tetris") == 0) {
          safe_print("Starting TETRIS...\n");

          tetris_main();
          /*
          int pid = task_create(tetris_main, 2);
          if (pid < 0) {
            safe_print("Failed to create TETRIS task! err code: ");
            safe_print_dec(-pid);
            safe_print("\n");
          } else {
            tcb_t *tetris_task = NULL;
            for (int i = 0; i < task_cnt; i++) {
              if (task_list[i] != NULL && task_list[i]->pid == pid) {
                tetris_task = task_list[i];
                break;
              }
            }
            if (tetris_task != NULL) {
              while ((*(volatile int *)&tetris_task->state) != STATE_DEAD) {
                enable_irq();
                sleep(0);
              }
            }
            for (volatile int k = 0; k < 500000; k++)
              ;


        }*/
          safe_print("\033[0m");
          safe_print("\033[?25h");
          safe_print("\033[2J");
          safe_print("\033[H");
          safe_print("TETRIS ended.\n");
        } else if (strcmp(cmd_buf, "gui") == 0) {
          enter_gui_mode();
          draw_box(10, 5, 1, 20, ANSI_COLOR_WHITE);
          draw_box(30, 5, 1, 20, ANSI_COLOR_WHITE);
          draw_box(10, 25, 11, 1, ANSI_COLOR_WHITE);

          draw_box(12, 20, 20, 2, ANSI_COLOR_RED);
          draw_box(16, 10, 2, 2, ANSI_COLOR_BLUE);
          gotoxy(40, 6);
          safe_print("Ghost-ARM GUI Mode");
          gotoxy(40, 8);
          safe_print("Press Q to exit GUI mode");

          while (1) {
            if ((*UART0FR & 0x10) == 0) {
              c = (char)(*UART0DR);
              if (c == 'q' || c == 'Q') {
                break;
              }
            }
          }
          exit_gui_mode();
        } else if (strcmp(cmd_buf, "uptime") == 0) {
          safe_print("Uptime: ");
          safe_print_dec(system_uptime / 100);
          safe_print(" seconds\n");
        } else if (strcmp(cmd_buf, "memtest") == 0) {
          safe_print("Allocating Memory...\n");
          char *mem1 = (char *)malloc(100);
          char *mem2 = (char *)malloc(200);
          char *mem3 = (char *)malloc(400);
          if (mem1 == 0 || mem2 == 0 || mem3 == 0) {
            safe_print("Memory allocation failed!\n");
          } else {
            safe_print("Allocated 256 bytes at: ");
            safe_print_dec((unsigned int)mem1);
            safe_print("\nAllocated 512 bytes at: ");
            safe_print_dec((unsigned int)mem2);
            safe_print("\nAllocated 1024 bytes at: ");
            safe_print_dec((unsigned int)mem3);
            safe_print("\n");
            mem1[0] = 'H';
            mem1[1] = 'i';
            mem1[2] = '\0';
            mem2[0] = 'G';
            mem2[1] = 'h';
            mem2[2] = 'o';
            mem2[3] = 's';
            mem2[4] = 't';
            mem2[5] = '\0';
            mem3[0] = 'O';
            mem3[1] = 'S';
            mem3[2] = '\0';
            safe_print("mem1 content: ");
            safe_print(mem1);
            safe_print("\n");
            safe_print("mem2 content: ");
            safe_print(mem2);
            safe_print("\n");
            safe_print("mem3 content: ");
            safe_print(mem3);
            safe_print("\n");
            free(mem1);
            free(mem2);
            free(mem3);
            safe_print("Memory freed.\n");
          }
        } else if (strcmp(cmd_buf, "jobs") == 0) {
          safe_print("Running tasks:\n");
          for (int i = 0; i < task_cnt; i++) {
            tcb_t *task = task_list[i];
            if (task->state == STATE_DEAD)
              continue;
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
          unsigned int target_pid = atoi(&cmd_buf[5]);
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
          unsigned int target_pid = atoi(&cmd_buf[5]);
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
          unsigned int target_pid = atoi(&cmd_buf[7]);
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
