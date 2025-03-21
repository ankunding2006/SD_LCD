/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file   fatfs.c
 * @brief  Code for fatfs applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
#include "fatfs.h"

uint8_t retSD;    /* Return value for SD */
char SDPath[4];   /* SD logical drive path */
FATFS SDFatFS;    /* File system object for SD logical drive */
FIL SDFile;       /* File object for SD */

/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

void MX_FATFS_Init(void)
{
  /*## FatFS: Link the SD driver ###########################*/
  retSD = FATFS_LinkDriver(&SD_Driver, SDPath);

  /* USER CODE BEGIN Init */
  /* additional user code for init */
  /* USER CODE END Init */
}

/**
  * @brief  Gets Time from RTC
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
  return 0;
  /* USER CODE END get_fattime */
}

/* USER CODE BEGIN Application */
// a simple function to test the SD card
/* USER CODE BEGIN Application */
// a simple function to test the SD card
void app_main(void)
{
  printf("Starting SD Card test...\r\n");
  app_fatfs();
  printf("SD Card test completed!\r\n");
}

void app_fatfs(void)
{
  FIL MyFile;
  FRESULT res;
  /* FatFs function common result code */
  uint32_t byteswritten, bytesread;                         /* File write/read counts */
  uint8_t wtext[] = "X Pulse STM32F407 working with FatFs"; /* File write buffer */
  uint8_t rtext[100];                                       /* File read buffer */
  //static uint8_t buffer[_MAX_SS];                           /* a work buffer for the f_mkfs() */

  /*##-2- Register the file system object to the FatFs module ##############*/
  if (f_mount(&SDFatFS, (TCHAR const *)SDPath, 0) != FR_OK)
  {
    /* FatFs Initialization Error */
    printf("FatFs mount error\r\n");
    Error_Handler();
  }
  else
  {
    /*##-3- Create a FAT file system (format) on the logical drive #########*/
//    res = f_mkfs((TCHAR const *)SDPath, FM_FAT32, 0, buffer, sizeof(buffer));
//    if (res != FR_OK)
//    {
//      /* FatFs Format Error */
//      printf("FatFs format error\r\n");
//      Error_Handler();
//    }
//    else
    {
      /*##-4- Create and Open a new text file object with write access #####*/
      printf("Creating file...\r\n");
      if (f_open(&MyFile, "STM32.TXT", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
      {
        /* 'STM32.TXT' file Open for write Error */
        printf("File creation error\r\n");
        Error_Handler();
      }
      else
      {
        /*##-5- Write data to the text file ################################*/
        printf("Writing to file...\r\n");
        res = f_write(&MyFile, wtext, sizeof(wtext), (void *)&byteswritten);

        if ((byteswritten == 0) || (res != FR_OK))
        {
          /* 'STM32.TXT' file Write or EOF Error */
          printf("File write error\r\n");
          Error_Handler();
        }
        else
        {
          /*##-6- Close the open text file #################################*/
          f_close(&MyFile);

          /*##-7- Open the text file object with read access ###############*/
          printf("Reading file...\r\n");
          if (f_open(&MyFile, "STM32.TXT", FA_READ) != FR_OK)
          {
            /* 'STM32.TXT' file Open for read Error */
            printf("File open for read error\r\n");
            Error_Handler();
          }
          else
          {
            /*##-8- Read data from the text file ###########################*/
            res = f_read(&MyFile, rtext, sizeof(rtext), (UINT *)&bytesread);

            //lcd显示读取的内容
            lcd_set_font(&lcd_desc, FONT_1608,BLUE, BLACK);
            lcd_print(&lcd_desc, 0, 150, "Read from file: %s", rtext);

            if ((bytesread == 0) || (res != FR_OK)) /* EOF or Error */
            {
              /* 'STM32.TXT' file Read or EOF Error */
              printf("File read error\r\n");
              Error_Handler();
            }
            else
            {
              /*##-9- Close the open text file #############################*/
              f_close(&MyFile);

              /*##-10- Compare read data with the expected data ############*/
              if ((bytesread != byteswritten))
              {
                /* Read data is different from the expected data */
                printf("Data verification failed\r\n");
                Error_Handler();
              }
              else
              {
                /* Success of the demo: no error occurrence */
                printf("SD Card test passed successfully!\r\n");
                led_on(); // 使用LED指示成功
              }
            }
          }
        }
      }
    }
  }
}
/* USER CODE END Application */
