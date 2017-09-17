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
#include "display.h"
#include "usart.h"
#include <stdlib.h>
#include <string.h>

FATFS SDDISKFatFs;  /* File system object for RAM disk logical drive */
FIL MyFile;          /* File object */
extern char SD_Path[];/* StartDefaultTask function */

FRESULT res;                                          /* FatFs function common result code */
uint32_t byteswritten, bytesread;                     /* File write/read counts */
DIR directoryObj;
uint8_t wtext[] = "This is STM32 working with FatFs"; /* File write buffer */
uint8_t rtext[1000];
int32_t readTextDeleteTimeout=0;
uint32_t g_file_read_finished_byte_number = 0;
uint32_t g_file_read_len, g_file_read_off;

char fil_dir_name[30];
char directoryContent[FILE_DIR_LIST_BUFFER_LEN];
char currentPath[FILE_MAX_PATH_LEN];

uint8_t currentRequest = 0;
uint8_t *requestStatusIndicator;

void debug_sendSerial(const char * msg) {
	return;
	int i;
	for(i=0; i<100; i++){
		if(msg[i] == '\0'){
			break;
		}
	}
	HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)msg, i);
}

FRESULT scan_files (
    char* path,       /* Start node to be scanned (also used as work area) */
	char *dest,
	uint32_t length,
	DIR *dir
)
{
    FRESULT res;
    FILINFO fno;

    int i;
    char *fn;   /* This function assumes non-Unicode configuration */
#if _USE_LFN
    static char lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */
    fno.lfname = lfn;
    fno.lfsize = sizeof lfn;
#endif

    if(path[strlen(path) - 1] == '/'){
    	path[strlen(path) - 1] = 0;
    }

    res = f_opendir(dir, path);                       /* Open the directory */
    if (res == FR_OK) {
    	i=strlen(dest);
        for(; i>=0;i--){
        	dest[i]=0;
        }
        i = 0;
        for (;;) {
            res = f_readdir(dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
#if _USE_LFN
            fn = *fno.lfname ? fno.lfname : fno.fname;
#else
            fn = fno.fname;
#endif
            if (fno.fname[0] == '.') {
            	i = strlen(dest);
            	snprintf(&dest[i], length-i, ".\n");
            } else if (fno.fattrib & AM_DIR) {                    /* It is a directory */
            	i = strlen(dest);
            	snprintf(&dest[i], length-i, "%s/..\n", fn);
            } else {                                       /* It is a file. */
            	i = strlen(dest);
            	snprintf(&dest[i], length-i, "%s\n", fn);
            }
        }
        f_closedir(dir);
    }

    if(path[strlen(path) - 1] != '/'){
		path[strlen(path)] = '/';
	}

    return res;
}

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

			if(res != FR_OK)
			{

				debug_sendSerial("fread result not ok\r");
				osDelay(100);


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

	strcpy(currentPath, SD_Path);
	scan_files(currentPath, directoryContent, FILE_DIR_LIST_BUFFER_LEN,&directoryObj);
	debug_sendSerial(directoryContent);


	while(1){

		vTaskGetInfo( xTaskGetCurrentTaskHandle(), &task_3, 1, eRunning);

		switch(currentRequest) {
		case READ_DATA_FROM_FILE:
			debug_sendSerial("read from file\r");
			if(f_open(&MyFile, fil_dir_name, FA_READ) != FR_OK) {
				debug_sendSerial(fil_dir_name);
				osDelay(10);
				debug_sendSerial(" - open file not ok\r");

			} else {
				debug_sendSerial(fil_dir_name);
				osDelay(10);
				debug_sendSerial(" - file open\r");
			}

			f_lseek(&MyFile, g_file_read_off);

			if(f_read(&MyFile, rtext, g_file_read_len, (UINT*)&bytesread) != FR_OK) {
				debug_sendSerial(fil_dir_name);
				osDelay(10);
				debug_sendSerial(" - read from file not ok\r");

			} else {
				debug_sendSerial(fil_dir_name);
				osDelay(10);
				debug_sendSerial(" - file read ok\r");
				g_file_read_finished_byte_number = bytesread;
			}

			f_close(&MyFile);
			*requestStatusIndicator = 1;
			requestStatusIndicator = 0;
			currentRequest = 0;
			readTextDeleteTimeout = 1000;
			break;
		case WRITE_DATA_TO_FILE:
			debug_sendSerial("write file\r");

			*requestStatusIndicator = 1;
			requestStatusIndicator = 0;
			currentRequest = 0;
			break;
		case GET_DIRECTORY_CONTENT:
			scan_files(currentPath, directoryContent, FILE_DIR_LIST_BUFFER_LEN,&directoryObj);
			debug_sendSerial("get dir content\r");
			*requestStatusIndicator = 1;
			requestStatusIndicator = 0;
			currentRequest = 0;
			break;
		case OPEN_DIRECTORY:
			debug_sendSerial("open dir\r");
			f_opendir(&directoryObj,currentPath);
			*requestStatusIndicator = 1;
			requestStatusIndicator = 0;
			currentRequest = 0;
			break;
		case CLOSE_CURRENT_DIRECTORY:
			debug_sendSerial("close dir\r");
			*requestStatusIndicator = 1;
			requestStatusIndicator = 0;
			currentRequest = 0;
			break;
		}

		osDelay(5);
		if(readTextDeleteTimeout>0) readTextDeleteTimeout -= 5;
		if(readTextDeleteTimeout == 0) {
			int i;
			for(i=0; i<1000; i++) {
				rtext[i] = 0;
			}
			readTextDeleteTimeout = -1;
		}
	}

}

uint8_t file_enterDirectory(char *dir_name, uint8_t *enterFinished) {
	if(requestStatusIndicator != NULL) {
		return 1;
	}
	currentRequest = OPEN_DIRECTORY;
	requestStatusIndicator = enterFinished;
	*enterFinished = 0;
	strcat(currentPath, dir_name);
	strcat(currentPath, "/");
	return 0;

}
void file_getDirectoryContent(char **content, uint32_t *length) {
	*content = directoryContent;
	*length = FILE_DIR_LIST_BUFFER_LEN;

}
uint8_t file_refreshDirectoryContent(uint8_t *refreshFinished) {
	if(requestStatusIndicator != NULL) {
		return 1;
	}
	currentRequest = GET_DIRECTORY_CONTENT;
	requestStatusIndicator = refreshFinished;
	*refreshFinished = 0;
	return 0;
}

void file_getFileContent(char **buffer, uint32_t *len) {
	*buffer = (char *)rtext;
	*len = g_file_read_finished_byte_number;
}
uint8_t file_readTextRequest(uint8_t *fileName, uint32_t offset, uint32_t length, uint8_t *readFinished) {
	if(requestStatusIndicator != NULL) {
		return 1;
	}
	currentRequest = READ_DATA_FROM_FILE;
	requestStatusIndicator = readFinished;
	*readFinished = 0;

	g_file_read_len = length;
	g_file_read_off = offset;

	strcpy((char*)fil_dir_name,(char*)currentPath);
	strcat((char*)fil_dir_name,(char*)fileName);
	return 0;
}

void file_getCurrentPath(char **path) {
	*path = currentPath;
}
void sd_openDir() {
	;
}
uint32_t file_getCurrentFileSize(void) {
	return f_size(&MyFile);
}
uint8_t file_isThisAFolderName(char *name) {
	uint32_t len = strlen(name);
	if(name[len-3] == '/' && name[len-2] == '.' && name[len-1] == '.'){
		return 1;
	} else {
		return 0;
	}
}

uint8_t file_isThisAOneLevelUpString(char *name) {
	uint32_t len = strlen(name);
	if(len == 2 && name[0] == '.' && name[1] == '.'){
		return 1;
	} else {
		return 0;
	}
}

void file_goOneLevelUp( void ) {
	uint32_t len = strlen(currentPath);
	int i;
	if(len == strlen("0:/")) {
		return;
	}
	currentPath[len-1] = 0;
	for(i=len-1; i>0;i--){
		if(currentPath[i] == '/') {
			break;
		}
		currentPath[i] = 0;
	}
}
