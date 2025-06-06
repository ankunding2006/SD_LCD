#ifndef LFS_PORT_H
#define LFS_PORT_H

#include "lfs.h"
#include <stdio.h>
#include <string.h>
#include <sfud.h>

// W25Q64 配置参数
#define LFS_BLOCK_SIZE 4096    // 4KB 扇区大小
#define LFS_BLOCK_COUNT 2048   // 8MB / 4KB = 2048个扇区
#define LFS_PROG_SIZE 256      // 页编程大小
#define LFS_READ_SIZE 256      // 读取大小
#define LFS_LOOKAHEAD_SIZE 128 // 空闲块查找缓冲区大小
#define LFS_CACHE_SIZE 256     // 缓存大小

// LittleFS接口函数声明
int lfs_flash_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
int lfs_flash_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
int lfs_flash_erase(const struct lfs_config *c, lfs_block_t block);
int lfs_flash_sync(const struct lfs_config *c);

// 全局配置结构
extern struct lfs_config lfs_cfg;
extern lfs_t lfs_fs;

// 缓冲区声明
extern uint8_t lfs_read_buffer[LFS_CACHE_SIZE];
extern uint8_t lfs_prog_buffer[LFS_CACHE_SIZE];
extern uint8_t lfs_lookahead_buffer[LFS_LOOKAHEAD_SIZE];

// LittleFS初始化和挂载函数
int lfs_port_init(void);
int lfs_port_mount(void);
int lfs_port_format(void);

void littlefs_demo(void);
void initialize_and_test_LittleFS(void);

#endif /* LFS_PORT_H */
