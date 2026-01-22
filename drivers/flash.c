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

void strcpy(char *dest, const char *src) {
  while (*src != '\0') {
    *dest = *src;
    dest++;
    src++;
  }
  *dest = '\0';
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

void flash_init() {
  *FLASH_BASE_ADDR = CMD_READ_ARRAY;
  safe_print("Flash memory initialized.\n");
}

void fs_format() {
  safe_print("Formatting Flash...\n");
  flash_erase_sector();
  safe_print("Format Done.\n");
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
  unsigned int offset = 0;
  while (1) {
    file_header_t *header = (file_header_t *)(FLASH_BASE_ADDR + offset / 4);
    volatile unsigned int *data_ptr = (volatile unsigned int *)header;
    if (header->magic != 0xA5A5A5A5) {
      break;
    }
    safe_print("File: ");
    safe_print(header->name);
    safe_print("[");
    safe_print_dec(header->size);
    safe_print(" bytes]: ");
    int val = data_ptr[sizeof(file_header_t) / 4];
    safe_print_dec(val);
    safe_print("\n");
    offset = header->next_offset;
    if (offset >= 0x4000000)
      break;
  }
  safe_print("---------------------------\n");
}

void fs_save_file(char *name, void *data, int size) {
  unsigned int offset = 0;
  while (1) {
    file_header_t *header = (file_header_t *)(FLASH_BASE_ADDR + offset / 4);
    if (header->magic != 0xA5A5A5A5) {
      break;
    }
    if (strcmp(header->name, name) == 0) {
      safe_print("File already exists. Overwrite not supported.\n");
      return;
    }
    offset = header->next_offset;
  }
  safe_print("Writing file to flash... Offset: ");
  safe_print_dec(offset);
  safe_print("\n");

  file_header_t new_header;
  new_header.magic = 0xA5A5A5A5;
  strcpy(new_header.name, name);
  new_header.size = size;

  unsigned int next = offset + HEADER_SIZE + size;
  // 4-byte alignment
  if (next % 4 != 0) {
    next += 4 - (next % 4);
  }
  new_header.next_offset = next;

  unsigned int *header_ptr = (unsigned int *)(&new_header);
  int header_words = HEADER_SIZE / 4;

  for (int i = 0; i < header_words; i++) {
    flash_write_word(offset / 4 + i, header_ptr[i]);
  }
  unsigned char *data_bytes = (unsigned char *)data;
  int data_start_idx = (offset / 4) + header_words;
  for (int i = 0; i < size; i += 4) {
    unsigned int pack = 0;
    for (int j = 0; j < 4; j++) {
      if (i + j < size) {
        pack |= ((unsigned int)(data_bytes[i + j])) << (j * 8);
      }
    }
    flash_write_word(data_start_idx + (i / 4), pack);
  }
  safe_print("File written successfully.\n");
}

int fs_load_file(char *name, void *buffer, int max_len) {
  unsigned int current_byte_offset = 0;
  while (1) {
    file_header_t *header =
        (file_header_t *)(FLASH_BASE_ADDR + (current_byte_offset / 4));
    if (header->magic != 0xA5A5A5A5)
      return -1;
    if (strcmp(header->name, name) == 0) {
      int copy_size = header->size;
      if (copy_size > max_len)
        copy_size = max_len;
      unsigned char *flash_data_ptr =
          (unsigned char *)header + sizeof(file_header_t);
      unsigned char *user_buf = (unsigned char *)buffer;

      for (int k = 0; k < copy_size; k++) {
        user_buf[k] = flash_data_ptr[k];
      }

      return header->size;
    }

    current_byte_offset = header->next_offset;
    if (current_byte_offset >= 0x4000000)
      return -1;
  }
}