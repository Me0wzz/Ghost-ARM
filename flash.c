#include "flash.h"
#include "ghost.h"

// Intel StrataFlash Commands
#define CMD_READ_ARRAY 0x00FF
#define CMD_BLOCK_ERASE 0x0020
#define CMD_CONFIRM 0x00D0
#define CMD_READ_STATUS 0x0070
#define CMD_CLEAR_STATUS 0x0050
#define CMD_PROGRAM 0x0040
#define STATUS_READY 1 << 7

void flash_init() {
  *FLASH_BASE_ADDR = CMD_READ_ARRAY;
  safe_print("Flash memory initialized.\n");
}

void flash_erase_sector() {
  volatile unsigned int *addr = FLASH_BASE_ADDR;
  *addr = CMD_BLOCK_ERASE;
  *addr = CMD_CONFIRM;
  while ((*addr & STATUS_READY) == 0)
    ;
  *FLASH_BASE_ADDR = CMD_READ_ARRAY;
}

void flash_write_word(int offset, int data) {
  volatile unsigned int *addr = FLASH_BASE_ADDR + offset;
  *addr = CMD_PROGRAM;
  *addr = data;
  while ((*addr & STATUS_READY) == 0)
    ;
  *FLASH_BASE_ADDR = CMD_READ_ARRAY;
}

void fs_save_score(int score) {
  safe_print("Saving score to flash...\n");
  flash_erase_sector();
  flash_write_word(0, 0x9999);
  flash_write_word(1, score);
  safe_print("Score saved.\n");
}
int fs_load_score() {
  volatile unsigned int *addr = FLASH_BASE_ADDR;
  if (addr[0] != 0x9999) {
    return 0;
  }
  return addr[1];
}

void fs_ls() {
  safe_print("--- Flash File Contents ---\n");
  volatile unsigned int *addr = FLASH_BASE_ADDR;
  if (addr[0] == 0x9999) {
    safe_print("[FILE] Best Score: ");
    safe_print_dec(addr[1]);
    safe_print("\n");
  } else {
    safe_print("Empty\n");
  }
}