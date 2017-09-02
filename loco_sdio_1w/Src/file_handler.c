/*
 * file_handler.c
 *
 *  Created on: 2017. szept. 2.
 *      Author: Lenovo
 */
#include "main.h"
#include "file_handler.h"
#include "fatfs.h"
#include "sdio.h"
#include "spi.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

FATFS SDDISKFatFs;  /* File system object for RAM disk logical drive */
FIL MyFile;          /* File object */
extern char SD_Path[];/* StartDefaultTask function */
FRESULT res;                                          /* FatFs function common result code */
uint32_t byteswritten, bytesread;                     /* File write/read counts */
uint8_t wtext[] = "This is STM32 working with FatFs"; /* File write buffer */
uint8_t rtext[100];

void debug_sendSerial(const char * msg) {
	int i;
	for(i=0; i<100; i++){
		if(msg[i] == '\0'){
			break;
		}
	}
	HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)msg, i);
}

FRESULT scan_files (
    char* path        /* Start node to be scanned (also used as work area) */
)
{
    FRESULT res;
    FILINFO fno;
    DIR dir;
    int i;
    char *fn;   /* This function assumes non-Unicode configuration */
#if _USE_LFN
    static char lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */
    fno.lfname = lfn;
    fno.lfsize = sizeof lfn;
#endif


    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        i = strlen(path);
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fname[0] == '.') continue;             /* Ignore dot entry */
#if _USE_LFN
            fn = *fno.lfname ? fno.lfname : fno.fname;
#else
            fn = fno.fname;
#endif
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                sprintf(&path[i], "/%s", fn);
                res = scan_files(path);
                path[i] = 0;
                if (res != FR_OK) break;
            } else {                                       /* It is a file. */
                printf("%s/%s\n", path, fn);
            }
        }
        f_closedir(&dir);
    }

    return res;
}
char path[100];
TaskStatus_t task_3;
void fileHandlerTask(void const * argument) {

	osDelay(100);
	/*##-2- Register the file system object to the FatFs module ##############*/
	while(f_mount(&SDDISKFatFs, "", 1/*(TCHAR const*)SD_Path, 0*/) != FR_OK)
	{
		debug_sendSerial("SD mount not OK\r");
		/* FatFs Initialization Error */
		Error_Handler();
	}


	debug_sendSerial("SD mount OK\r");
	osDelay(100);
//		if(f_mkfs((TCHAR const*)SD_Path, 0, 0) != FR_OK)
//		{
//			HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"Make fs not OK\r", 15);
//			osDelay(100);
//			/* FatFs Format Error */
//			Error_Handler();
//		}
//		else

	debug_sendSerial("Make fs OK\r");
	osDelay(100);

			/*##-4- Create and Open a new text file object with write access #####*/
	while(f_open(&MyFile, "STM32asd.TXT", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
	{
		debug_sendSerial("fopen not OK\r");
		/* 'STM32.TXT' file Open for write Error */
		Error_Handler();
	}

	debug_sendSerial("fopen OK\r");
	 osDelay(100);

	 /*##-5- Write data to the text file ################################*/
	 res = f_write(&MyFile, wtext, sizeof(wtext), (void *)&byteswritten);

	if((byteswritten == 0) || (res != FR_OK))
	{
		debug_sendSerial("fwrite not OK\r");
		 osDelay(100);

		/* 'STM32.TXT' file Write or EOF Error */
		Error_Handler();
	}
	else
	{
		debug_sendSerial("fwrite OK\r");
		osDelay(100);
		/*##-6- Close the open text file #################################*/
		f_close(&MyFile);
		debug_sendSerial("File closed, read back!\r");
		osDelay(100);


		/*##-7- Open the text file object with read access ###############*/
		if(f_open(&MyFile, "STM32asd.TXT", FA_READ) != FR_OK)
		{
			debug_sendSerial("fopen not OK\r");
			osDelay(100);

			/* 'STM32.TXT' file Open for read Error */
			Error_Handler();
		}
		else
		{
			debug_sendSerial("fopen OK\r");
			osDelay(100);
			/*##-8- Read data from the text file ###########################*/
			res = f_read(&MyFile, rtext, sizeof(rtext), (void *)&bytesread);

			if((bytesread == 0) || (res != FR_OK))
			{
				if(bytesread == 0) {
					debug_sendSerial("0 bytes read back error\r");
					   osDelay(100);
				} else {
					debug_sendSerial("fread result not ok\r");
					   osDelay(100);
				}

				/* 'STM32.TXT' file Read or EOF Error */
				Error_Handler();
			}
			else
			{
				/*##-9- Close the open text file #############################*/
				f_close(&MyFile);

				/*##-10- Compare read data with the expected data ############*/
				if((bytesread != byteswritten))
				{
					debug_sendSerial("Read file not correct\r");
					osDelay(100);
					/* Read data is different from the expected data */
					Error_Handler();
				}
				else
				{
					debug_sendSerial("SD test success\r");
					osDelay(100);

					/* Success of the demo: no error occurrence */
					bytesread = 0;
				}
			}
		}
	}
	path[0] = '0';
	path[1] = ':';
	path[2] = '/';
	path[3] = 'a';
	path[4] = 'a';
	path[5] = 'a';
	path[6] = '/';
	path[7] = '\0';
	scan_files(path);
	path[0] = '0';
		path[1] = ':';
		path[2] = '/';
		path[3] = '\0';
	scan_files(path);
	path[0] = '0';
		path[1] = ':';
		path[2] = '/';
		path[3] = '\0';
	scan_files(path);
	path[0] = '0';
		path[1] = ':';
		path[2] = '/';
		path[3] = '\0';
	scan_files(path);
	path[0] = '0';
		path[1] = ':';
		path[2] = '/';
		path[3] = '\0';
	scan_files(path);
	scan_files(path);
	scan_files(path);


	while(1){

		vTaskGetInfo( xTaskGetCurrentTaskHandle(), &task_3, 1, eRunning);

//		switch(currentRequest) {
//		case READ_DATA_FROM_FILE:
//			debug_sendSerial("read from file\r");
//			break;
//		case WRITE_DATA_TO_FILE:
//			debug_sendSerial("write file\r");
//			break;
//		case GET_DIRECTORY_CONTENT:
//			debug_sendSerial("get dir content\r");
//			break;
//		case OPEN_DIRECTORY:
//			debug_sendSerial("open dir\r");
//			break;
//		case CLOSE_CURRENT_DIRECTORY:
//			debug_sendSerial("close dir\r");
//			break;
//		}

		osDelay(100);
	}

}

void sd_read(uint8_t *fileName, uint32_t offset, uint32_t length, uint8_t *buffer) {
	uint32_t read_bytes;
	if(f_open(&MyFile, fileName, FA_READ) != FR_OK) {
		debug_sendSerial(fileName);
		osDelay(10);
		debug_sendSerial(" - open file not ok\r");
		return;
	} else {
		debug_sendSerial(fileName);
		osDelay(10);
		debug_sendSerial(" - file open\r");
	}

	f_lseek(&MyFile, offset);

	if(f_read(&MyFile, buffer, length, &read_bytes) != FR_OK) {
		debug_sendSerial(fileName);
		osDelay(10);
		debug_sendSerial(" - read from file not ok\r");
		return;
	} else {
		debug_sendSerial(fileName);
		osDelay(10);
		debug_sendSerial(" - file read ok\r");
	}
}

void sd_openDir() {
	;
}
