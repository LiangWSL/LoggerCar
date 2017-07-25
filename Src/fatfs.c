/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#include "fatfs.h"
#include "exFlash.h"

uint8_t retUSER;    /* Return value for USER */
char USER_Path[4];  /* USER logical drive path */

/* USER CODE BEGIN Variables */
FATFS FATFS_exFlashObj;				/* �ļ�ϵͳ���� */
FIL   fileObj;						/* �ļ����� */
FRESULT optStatus;					/* ����״̬ */
UINT  optNumb;						/* �ļ��������� */
BYTE  readBuf[100];
BYTE  writeBuf[] = {"hangzhoulugekejiyouxiangongsi"};
BYTE  writeBuf2[] = {"Zhejiang.Hangzhou"};

/* USER CODE END Variables */    

void MX_FATFS_Init(void) 
{
  /*## FatFS: Link the USER driver ###########################*/
  retUSER = FATFS_LinkDriver(&USER_Driver, USER_Path);

  /* USER CODE BEGIN Init */
  /* additional user code for init */   
  if (0 == retUSER)
  {
	  printf("Ӳ�������ӳɹ�\r\n");
	  exFLASH_SectorErase(0);
	  optStatus = f_mount(&FATFS_exFlashObj, USER_Path, 1);
	  if (optStatus == FR_NO_FILESYSTEM)
	  {
		  printf("�������и�ʽ��\r\n");
		  exFLASH_ChipErase();
		  optStatus = f_mkfs(USER_Path, 1, 4096);
		  if (optStatus == FR_OK)
		  {
			  printf("��ʽ���ɹ�\r\n");
			  f_mount(NULL, USER_Path, 1);
			  f_mount(&FATFS_exFlashObj, USER_Path, 1);
		  }
		  else
		  {
			  printf("��ʽ��ʧ��\r\n");
		  }
	  }

	  optStatus = f_open(&fileObj, "stm32.txt", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
	  if (optStatus == FR_OK)
	  {
		  printf("�ļ��򿪳ɹ�\r\n");
		  optStatus = f_write(&fileObj, writeBuf, sizeof(writeBuf), &optNumb);
		  if(optStatus == FR_OK)
		  {
			  printf("�ļ�д��ɹ�\r\n");
		  }
		  else
		  {
			  printf("�ļ�д��ʧ��\r\n");
		  }
		  optStatus = f_read(&fileObj, readBuf, sizeof(readBuf), &optNumb);
		  f_close(&fileObj);
	  }
	  else
	  {
		  printf("�ļ���ʧ��\r\n");
	  }

	  optStatus = f_open(&fileObj, "stm32.txt", FA_READ);
	  if (FR_OK == optStatus)
	  {
		  optStatus = f_read(&fileObj, readBuf, sizeof(readBuf), &optNumb);
//		  optStatus = f_write(&fileObj, writeBuf2, sizeof(writeBuf2), &optNumb);
//		  f_close(&fileObj);
//
//		  optStatus = f_open(&fileObj, "stm32.txt", FA_OPEN_EXISTING | FA_READ);
//		  optStatus = f_read(&fileObj, readBuf, sizeof(readBuf), &optNumb);
		  f_close(&fileObj);
	  }


  }
  FATFS_UnLinkDriver(USER_Path);
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
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
