#ifndef FS_H
#define FS_H

// VersatilePB Flash Memory Base Address
#define FLASH_BASE_ADDR ((volatile unsigned int *)0x34000000)

void flash_init();
void fs_save_score(int score);
int fs_load_score();
void fs_ls();

#endif