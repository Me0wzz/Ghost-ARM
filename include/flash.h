#ifndef FS_H
#define FS_H

#define MAX_FILENAME_LEN 20

typedef struct {
  unsigned int magic;
  char name[MAX_FILENAME_LEN];
  unsigned int size;
  unsigned int next_offset;
  unsigned int padding;
} file_header_t;

// VersatilePB Flash Memory Base Address
#define FLASH_BASE_ADDR ((volatile unsigned int *)0x34000000)
#define HEADER_SIZE sizeof(file_header_t)

void flash_init();
void fs_format();
void fs_save_file(char *name, void *data, int size);
int fs_load_file(char *name, void *buffer, int max_len);
void fs_ls();

#endif