#include "ghost.h"

void clrscr() {
  safe_print("\033[1;1H");      // Move cursor to (1, 1) position
  safe_print("\033[2J\033[3J"); // Clear screen and scrollback buffer
}

void set_color(int color) {
  safe_print("\033[");
  safe_print_dec(color);
  safe_print("m");
}

void gotoxy(int x, int y) {
  safe_print("\033[");
  safe_print_dec(y);
  safe_print(";");
  safe_print_dec(x);
  safe_print("H");
}

void draw_box(int x, int y, int w, int h, int color) {
  set_color(color);
  for (int i = 0; i < h; i++) {
    gotoxy(x, y + i);
    for (int j = 0; j < w; j++) {
      safe_print("[]");
    }
  }
  safe_print("\n");
  set_color(ANSI_COLOR_RESET);
}

void enter_gui_mode() {
  safe_print("\033[?1049h"); // Switch to alternate screen buffer"
  safe_print("\033[?25l");   // Hide cursor
  safe_print("\033[2J");     // Clear screen
  safe_print("\033[H");      // Move cursor to (1, 1)
}

void exit_gui_mode() {
  safe_print("\033[0m"); // Reset colors
  safe_print("\033[H\033[2J\033[3J");
  safe_print("\033[?25h");   // Show cursor
  safe_print("\033[?1049l"); // Switch back to normal screen buffer
}