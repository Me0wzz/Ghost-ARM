#include "ghost.h"

#define BOARD_W 10
#define BOARD_H 20

extern void fs_save_score(int score);
extern int fs_load_score();

int board[BOARD_H][BOARD_W];
int px = 3, py = 0, shape = 0;
int score = 0;
int best_score = 0;

char current_block[4][4];

char shapes[7][4][4] = {
    {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // I
    {{0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}}, // O
    {{0, 0, 0, 0}, {0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}}, // T
    {{0, 0, 0, 0}, {0, 0, 1, 1}, {0, 1, 1, 0}, {0, 0, 0, 0}}, // S
    {{0, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}}, // Z
    {{0, 0, 0, 0}, {1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}}, // J
    {{0, 0, 0, 0}, {0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}}  // L
};

int shape_colors[7] = {ANSI_COLOR_CYAN,  ANSI_COLOR_YELLOW, ANSI_COLOR_MAGENTA,
                       ANSI_COLOR_GREEN, ANSI_COLOR_RED,    ANSI_COLOR_BLUE,
                       ANSI_COLOR_WHITE};

int detect_collision(int px, int py, char block[4][4]) {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (block[y][x]) { //
        int nx = px + x;
        int ny = py + y;
        if (nx < 0 || nx >= BOARD_W || ny < 0 || ny >= BOARD_H)
          return 1;
        if (ny >= 0 && board[ny][nx] != 0)
          return 1;
      }
    }
  }
  return 0;
}

void draw_ui() {
  gotoxy(45, 5);
  safe_print("====== SCORE / BEST SCORE ======");
  gotoxy(45, 7);
  safe_print("          ");
  safe_print_dec(score);
  safe_print(" / ");
  safe_print_dec(best_score);
  safe_print("          ");
  gotoxy(45, 9);

  gotoxy(45, 11);
  safe_print("Space : Drop");
  gotoxy(45, 12);
  safe_print("A/D : Move Left/Right");
  gotoxy(45, 13);
  safe_print("S : Move Down");
  gotoxy(45, 14);
  safe_print("R : Rotate");
  gotoxy(45, 15);
  safe_print("Q : Quit Game");
}

void copy_block(int shape_idx) {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      current_block[y][x] = shapes[shape_idx][y][x];
    }
  }
}

void rotate_block() {
  char temp[4][4];
  // 90degree rotation: new[x][y] = old[3-y][x]
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      temp[y][x] = current_block[3 - x][y];
    }
  }
  if (!detect_collision(px, py, temp)) {
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 4; x++) {
        current_block[y][x] = temp[y][x];
      }
    }
    return;
  }
  if (!detect_collision(px + 1, py, temp)) {
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 4; x++) {
        current_block[y][x] = temp[y][x];
      }
    }
    return;
  }
  if (!detect_collision(px - 1, py, temp)) {
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 4; x++) {
        current_block[y][x] = temp[y][x];
      }
    }
    return;
  }
}

void draw_background() {
  safe_print("\033[?25l"); // Hide cursor
  for (int y = 0; y < BOARD_H; y++) {
    gotoxy(20, y + 5);
    safe_print("|");
    for (int x = 0; x < BOARD_W; x++) {
      safe_print("  ");
    }
    safe_print("|");
  }
  gotoxy(20, BOARD_H + 5);
  safe_print("+");
  for (int x = 0; x < BOARD_W * 2; x++)
    safe_print("-");
  safe_print("+");
}

void draw_block(int px, int py, int shape_idx, int mode) {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (current_block[y][x]) {
        int nx = px + x;
        int ny = py + y;
        if (nx >= 0 && nx < BOARD_W && ny >= 0 && ny < BOARD_H) {
          gotoxy(21 + nx * 2, 5 + ny);
          if (mode == 1) {
            set_color(shape_colors[shape_idx]);
            safe_print("[]");
          } else {
            set_color(ANSI_COLOR_RESET);
            safe_print("  ");
          }
        }
      }
    }
  }
  set_color(ANSI_COLOR_RESET);
}

void redraw() {
  for (int y = 0; y < BOARD_H; y++) {
    for (int x = 0; x < BOARD_W; x++) {
      gotoxy(21 + x * 2, 5 + y);
      if (board[y][x]) {
        set_color(board[y][x]);
        safe_print("[]");
      } else {
        set_color(ANSI_COLOR_RESET);
        safe_print("  ");
      }
    }
  }
  set_color(ANSI_COLOR_RESET);
}

void tetris_main() {
  score = 0;
  px = 3;
  py = 0;
  shape = 0;
  int saved = fs_load_score();
  if (saved != -1) {
    best_score = saved;
  }
  enter_gui_mode();
  draw_background();
  draw_ui();
  copy_block(shape);
  draw_block(px, py, shape, 1);
  unsigned int current_time, last_drop_time = system_uptime;
  for (int i = 0; i < BOARD_H; i++)
    for (int j = 0; j < BOARD_W; j++)
      board[i][j] = 0;
  while (1) {
    current_time = system_uptime;
    while ((*UART0FR & 0x10) == 0) {
      char c = (char)(*UART0DR);
      if (c == 'q' || c == 'Q') {
        goto game_end;
      }
      draw_block(px, py, shape, 0);
      int new_px = px, new_py = py;
      if (c == 'r' || c == 'R') {
        rotate_block(px, py);
      } else if (c == ' ') {
        while (!detect_collision(new_px, py + 1, current_block)) {
          py++;
        }
        last_drop_time = current_time - 1000;
        new_py = py;
      }

      else {
        if (c == 'a' || c == 'A') {
          new_px--;
        } else if (c == 'd' || c == 'D') {
          new_px++;
        } else if (c == 's' || c == 'S') {
          new_py++;
        }
      }

      if (!detect_collision(new_px, new_py, current_block)) {

        px = new_px;
        py = new_py;
      }
      draw_block(px, py, shape, 1);
    }
    if (current_time - last_drop_time >= 150) {
      last_drop_time = current_time;
      if (!detect_collision(px, py + 1, current_block)) {
        draw_block(px, py, shape, 0);
        py++;
        draw_block(px, py, shape, 1);
      } else {
        for (int y = 0; y < 4; y++) {
          for (int x = 0; x < 4; x++) {
            if (current_block[y][x]) {
              int nx = px + x;
              int ny = py + y;
              if (nx >= 0 && nx < BOARD_W && ny >= 0 && ny < BOARD_H) {
                board[ny][nx] = shape_colors[shape];
              }
            }
          }
        }
        int line = 0;
        for (int y = BOARD_H - 1; y >= 0; y--) {
          int full = 1;
          for (int x = 0; x < BOARD_W; x++) {
            if (board[y][x] == 0) {
              full = 0;
              break;
            }
          }
          if (full) {
            for (int k = y; k > 0; k--) {
              for (int x = 0; x < BOARD_W; x++) {
                board[k][x] = board[k - 1][x];
              }
            }
            y++;
            line++;
          }
        }
        if (line > 0) {
          score += (line * line) * 100;
          redraw();
          draw_ui();
        }
        px = 3, py = 0, shape = (system_uptime + px) % 7;
        redraw();
        copy_block(shape);
        if (detect_collision(px, py, current_block)) {
          break;
        }
        draw_block(px, py, shape, 1);
      }
    }
  }
game_end:

  exit_gui_mode();
  for (volatile int i = 0; i < 3000000; i++)
    ;

  // current_task->state = STATE_DEAD;
  if (score > best_score) {
    best_score = score;
    fs_save_score(best_score);
  }
  safe_print("Game Over! Your Score: ");
  safe_print_dec(score);
  safe_print("\nBest Score: ");
  safe_print_dec(best_score);
  safe_print("\n");
  return;
}