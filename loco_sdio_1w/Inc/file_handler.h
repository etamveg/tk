/*
 * file_handler.h
 *
 *  Created on: 2017. szept. 2.
 *      Author: Lenovo
 */

#ifndef FILE_HANDLER_H_
#define FILE_HANDLER_H_

#include <stdint.h>

#define READ_DATA_FROM_FILE			0x01
#define WRITE_DATA_TO_FILE			0x02
#define GET_DIRECTORY_CONTENT		0x04
#define OPEN_DIRECTORY				0x08
#define CLOSE_CURRENT_DIRECTORY		0x10

#define FILE_DIR_LIST_BUFFER_LEN 	300
#define FILE_MAX_PATH_LEN 			64

void fileHandlerTask(void const * argument);

void file_getDirectoryContent(char **content, uint32_t *length);
uint8_t file_refreshDirectoryContent(uint8_t *refreshFinished);
void file_getFileContent(char **buffer, uint32_t *len);
uint8_t file_readTextRequest(uint8_t *fileName, uint32_t offset, uint32_t length, uint8_t *readFinished);
void file_getCurrentPath(char **path);
uint8_t file_refreshDirectoryContent(uint8_t *refreshFinished);
void file_getDirectoryContent(char **content, uint32_t *length);
void file_getCurrentPath(char **path);
uint8_t file_enterDirectory(char *dir_name, uint8_t *enterFinished);
uint8_t file_isThisAFolderName(char *name);
uint8_t file_isThisAOneLevelUpString(char *name);
void file_goOneLevelUp( void );

#endif /* FILE_HANDLER_H_ */
