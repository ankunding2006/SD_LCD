#include "lfs_port.h"
#include "sfud.h"
#include <string.h>

// SFUD缓冲区
#define SFUD_DEMO_TEST_BUFFER_SIZE 1024
static uint8_t sfud_demo_test_buf[SFUD_DEMO_TEST_BUFFER_SIZE];

// LittleFS缓冲区定义
uint8_t lfs_read_buffer[LFS_CACHE_SIZE];
uint8_t lfs_prog_buffer[LFS_CACHE_SIZE];
uint8_t lfs_lookahead_buffer[LFS_LOOKAHEAD_SIZE];

// LittleFS文件系统实例
lfs_t lfs_fs;

// SFUD Flash设备指针
static const sfud_flash *flash = NULL;
/**
 * @brief 断言失败时的回调函数
 *
 * @param expr 失败的表达式
 * @param file 失败的文件
 * @param line 失败的行号
 */
void __aeabi_assert(const char *expr, const char *file, int line)
{
  // 简单的assert实现，可以根据需要修改
  (void)expr;
  (void)file;
  (void)line;
  // 在调试模式下可以添加打印或断点
  while (1)
    ; // 进入无限循环，或者可以重置系统
}

/**
 * @brief LittleFS读取函数
 * @param c LittleFS配置结构指针
 * @param block 块号
 * @param off 块内偏移
 * @param buffer 读取数据缓冲区
 * @param size 读取数据大小
 * @return 0成功，负值失败
 */
int lfs_flash_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
  if (flash == NULL)
  {
    flash = sfud_get_device(SFUD_W25_DEVICE_INDEX);
    if (flash == NULL)
    {
      return LFS_ERR_IO;
    }
  }

  // 计算实际地址：块号 * 块大小 + 块内偏移
  uint32_t addr = block * c->block_size + off;

  // 使用SFUD读取数据
  sfud_err result = sfud_read(flash, addr, size, (uint8_t *)buffer);

  return (result == SFUD_SUCCESS) ? LFS_ERR_OK : LFS_ERR_IO;
}

/**
 * @brief LittleFS编程（写入）函数
 * @param c LittleFS配置结构指针
 * @param block 块号
 * @param off 块内偏移
 * @param buffer 写入数据缓冲区
 * @param size 写入数据大小
 * @return 0成功，负值失败
 */
int lfs_flash_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
  if (flash == NULL)
  {
    flash = sfud_get_device(SFUD_W25_DEVICE_INDEX);
    if (flash == NULL)
    {
      return LFS_ERR_IO;
    }
  }

  // 计算实际地址：块号 * 块大小 + 块内偏移
  uint32_t addr = block * c->block_size + off;

  // 使用SFUD写入数据
  sfud_err result = sfud_write(flash, addr, size, (const uint8_t *)buffer);

  return (result == SFUD_SUCCESS) ? LFS_ERR_OK : LFS_ERR_IO;
}

/**
 * @brief LittleFS擦除函数
 * @param c LittleFS配置结构指针
 * @param block 要擦除的块号
 * @return 0成功，负值失败
 */
int lfs_flash_erase(const struct lfs_config *c, lfs_block_t block)
{
  if (flash == NULL)
  {
    flash = sfud_get_device(SFUD_W25_DEVICE_INDEX);
    if (flash == NULL)
    {
      return LFS_ERR_IO;
    }
  }

  // 计算实际地址：块号 * 块大小
  uint32_t addr = block * c->block_size;

  // 使用SFUD擦除扇区（4KB）
  sfud_err result = sfud_erase(flash, addr, c->block_size);

  return (result == SFUD_SUCCESS) ? LFS_ERR_OK : LFS_ERR_IO;
}

/**
 * @brief LittleFS同步函数
 * @param c LittleFS配置结构指针
 * @return 0成功，负值失败
 */
int lfs_flash_sync(const struct lfs_config *c)
{
  // Flash操作是同步的，不需要额外操作
  return LFS_ERR_OK;
}

// LittleFS配置结构
struct lfs_config lfs_cfg = {
    // 块设备操作函数
    .read = lfs_flash_read,
    .prog = lfs_flash_prog,
    .erase = lfs_flash_erase,
    .sync = lfs_flash_sync,

    // 块设备配置
    .read_size = LFS_READ_SIZE,
    .prog_size = LFS_PROG_SIZE,
    .block_size = LFS_BLOCK_SIZE,
    .block_count = LFS_BLOCK_COUNT,
    .cache_size = LFS_CACHE_SIZE,
    .lookahead_size = LFS_LOOKAHEAD_SIZE,
    .block_cycles = 500,

    // 可选缓冲区
    .read_buffer = lfs_read_buffer,
    .prog_buffer = lfs_prog_buffer,
    .lookahead_buffer = lfs_lookahead_buffer,
};

/**
 * @brief 初始化LittleFS移植层
 * @return 0成功，负值失败
 */
int lfs_port_init(void)
{
  // 初始化SFUD
  if (sfud_init() != SFUD_SUCCESS)
  {
    return -1;
  }

  // 获取Flash设备
  flash = sfud_get_device(SFUD_W25_DEVICE_INDEX);
  if (flash == NULL)
  {
    return -1;
  }

  return 0;
}

/**
 * @brief 挂载LittleFS文件系统
 * @return 0成功，负值失败
 */
int lfs_port_mount(void)
{
  // 挂载文件系统
  int err = lfs_mount(&lfs_fs, &lfs_cfg);

  // 如果挂载失败，可能需要格式化
  if (err)
  {
    // 格式化文件系统
    lfs_format(&lfs_fs, &lfs_cfg);

    // 重新挂载
    err = lfs_mount(&lfs_fs, &lfs_cfg);
  }

  return err;
}

/**
 * @brief 格式化LittleFS文件系统
 * @return 0成功，负值失败
 */
int lfs_port_format(void)
{
  return lfs_format(&lfs_fs, &lfs_cfg);
}

/**
 * @brief LittleFS演示函数
 * @retval None
 */
void littlefs_demo(void)
{
  int err;
  lfs_file_t file;
  const char *test_data = "Hello LittleFS! This is a test file.";
  char read_buffer[100] = {0};
  lfs_size_t written = 0;    // 预先初始化
  lfs_ssize_t read_size = 0; // 使用有符号类型进行错误检查

  printf("\r\n=== LittleFS Demo Start ===\r\n");

  // 初始化LittleFS移植层
  err = lfs_port_init();
  if (err < 0)
  {
    printf("LittleFS port init failed: %d\r\n", err);
    return;
  }
  printf("LittleFS port init success\r\n");

  // 挂载文件系统
  err = lfs_port_mount();
  if (err < 0)
  {
    printf("LittleFS mount failed: %d\r\n", err);
    return;
  }
  printf("LittleFS mount success\r\n");
  // 创建并写入测试文件
  err = lfs_file_open(&lfs_fs, &file, "test.txt", LFS_O_WRONLY | LFS_O_CREAT);
  if (err < 0)
  {
    printf("File open for write failed: %d\r\n", err);
    goto cleanup;
  }

  written = lfs_file_write(&lfs_fs, &file, test_data, strlen(test_data));
  if (written != strlen(test_data))
  {
    printf("File write failed: written %d, expected %d\r\n", (int)written, (int)strlen(test_data));
    lfs_file_close(&lfs_fs, &file);
    goto cleanup;
  }
  printf("File write success: %d bytes\r\n", (int)written);

  lfs_file_close(&lfs_fs, &file);

  // 读取测试文件
  err = lfs_file_open(&lfs_fs, &file, "test.txt", LFS_O_RDONLY);
  if (err < 0)
  {
    printf("File open for read failed: %d\r\n", err);
    goto cleanup;
  }

  read_size = lfs_file_read(&lfs_fs, &file, read_buffer, sizeof(read_buffer) - 1);
  if (read_size < 0)
  {
    printf("File read failed: %d\r\n", (int)read_size);
    lfs_file_close(&lfs_fs, &file);
    goto cleanup;
  }

  read_buffer[read_size] = '\0'; // 确保字符串结束
  printf("File read success: %d bytes\r\n", (int)read_size);
  printf("File content: %s\r\n", read_buffer);

  lfs_file_close(&lfs_fs, &file);

  // 验证数据
  if (strcmp(test_data, read_buffer) == 0)
  {
    printf("Data verification success!\r\n");
  }
  else
  {
    printf("Data verification failed!\r\n");
  }
  // 获取文件系统状态
  struct lfs_info info;
  err = lfs_stat(&lfs_fs, "test.txt", &info);
  if (err >= 0)
  {
    printf("File size: %d bytes\r\n", (int)info.size);
    printf("File type: %s\r\n", (info.type == LFS_TYPE_REG) ? "Regular file" : "Directory");
  }

  // 列出根目录文件
  lfs_dir_t dir;
  err = lfs_dir_open(&lfs_fs, &dir, "/");
  if (err >= 0)
  {
    printf("Root directory contents:\r\n");
    while (true)
    {
      err = lfs_dir_read(&lfs_fs, &dir, &info);
      if (err < 0)
        break;
      if (err == 0)
        break; // 没有更多文件      if (info.name[0] != '.')
      {        // 跳过 . 和 .. 目录
        printf("  %s (%s, %d bytes)\r\n",
               info.name,
               (info.type == LFS_TYPE_REG) ? "file" : "dir",
               (int)info.size);
      }
    }
    lfs_dir_close(&lfs_fs, &dir);
  }

cleanup:
  printf("=== LittleFS Demo End ===\r\n\r\n");
}

void sfud_demo(uint32_t addr, size_t size, uint8_t *data)
{
  sfud_err result = SFUD_SUCCESS;
  extern sfud_flash *sfud_dev;
  const sfud_flash *flash = sfud_get_device(SFUD_W25_DEVICE_INDEX);
  size_t i;
  /* prepare write data */
  for (i = 0; i < size; i++)
  {
    data[i] = i;
  }
  /* erase test */
  result = sfud_erase(flash, addr, size);
  if (result == SFUD_SUCCESS)
  {
    printf("Erase the %s flash data finish. Start from 0x%08X, size is %zu.\r\n", flash->name, addr, size);
  }
  else
  {
    printf("Erase the %s flash data failed.\r\n", flash->name);
    return;
  }
  /* write test */
  result = sfud_write(flash, addr, size, data);
  if (result == SFUD_SUCCESS)
  {
    printf("Write the %s flash data finish. Start from 0x%08X, size is %zu.\r\n", flash->name, addr, size);
  }
  else
  {
    printf("Write the %s flash data failed.\r\n", flash->name);
    return;
  }
  /* read test */
  result = sfud_read(flash, addr, size, data);
  if (result == SFUD_SUCCESS)
  {
    printf("Read the %s flash data success. Start from 0x%08X, size is %zu. The data is:\r\n", flash->name, addr, size);
    printf("Offset (h) 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\r\n");
    for (i = 0; i < size; i++)
    {
      if (i % 16 == 0)
      {
        printf("[%08X] ", addr + i);
      }
      printf("%02X ", data[i]);
      if (((i + 1) % 16 == 0) || i == size - 1)
      {
        printf("\r\n");
      }
    }
    printf("\r\n");
  }
  else
  {
    printf("Read the %s flash data failed.\r\n", flash->name);
  }
  /* data check */
  for (i = 0; i < size; i++)
  {
    if (data[i] != i % 256)
    {
      printf("Read and check write data has an error. Write the %s flash data failed.\r\n", flash->name);
      break;
    }
  }
  if (i == size)
  {
    printf("The %s flash test is success.\r\n", flash->name);
  }
}

/**
 * @brief 初始化并测试LittleFS和SFUD
 *
 */
void initialize_and_test_LittleFS(void)
{
  if (sfud_init() == SFUD_SUCCESS)
  {
    sfud_demo(0, sizeof(sfud_demo_test_buf), sfud_demo_test_buf);

    // 运行LittleFS演示
    littlefs_demo();
  }
}
