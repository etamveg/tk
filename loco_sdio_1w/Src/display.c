/*
 * display.c
 *
 *  Created on: 2017. aug. 30.
 *      Author: Lenovo
 */

#include "display.h"
#include "fonts.h"
#include "spi.h"
#include "cmsis_os.h"
uint8_t displayData[256*4];
uint8_t displayGrayscale = 0xf;

dsp_textbox_t g_textbox_list[2];

void dsp_setPixelToMemory(uint8_t *data, uint8_t x, uint8_t y);
void dsp_clearPixelFromMemory(uint8_t *data, uint8_t x, uint8_t y);
void dsp_writeDataToDisplay(uint8_t *data, uint8_t grayScale);
void dsp_printCharacterToMemory(uint8_t *data, uint8_t font, uint8_t base_x, uint8_t base_y, uint8_t char_offset);
void dsp_displayInitWithCommands(void);
uint32_t dsp_calculateCharTableOffsetOfChar(uint8_t character);

void dsp_setPixelToMemory(uint8_t *data, uint8_t x, uint8_t y) {
	uint32_t pos;
	if(x>=256 || y>=32) {
		return;
	}
	pos = x+y*256;
	data[pos/8] |= (1 << (pos%8));
}
void dsp_clearPixelFromMemory(uint8_t *data, uint8_t x, uint8_t y) {
	uint32_t pos;
	if(x>=256 || y>=32) {
		return;
	}
	pos = x+y*256;
	data[pos/8] &= ~(1 << (pos%8));
}
void dsp_writeDisplayCmd(uint8_t cmd){
	HAL_SPI_SS(DISPLAY_SELECTED);
	//osDelay(1);
	HAL_SPI_Transmit(&hspi1, &cmd, 1, 0);
	//osDelay(1);
	//HAL_SPI_SS(DISPLAY_NOT_SELECTED);
}
void dsp_writeDataToDisplay(uint8_t *data, uint8_t grayScale){
	int  j;
	uint8_t byte_to_send;
	if(grayScale>15) {
		grayScale = 15;
	}
	for(j=0;j<256*32;j+=2){//lines

		byte_to_send = 0;
		byte_to_send |= (data[j/8] & (1<<(j%8))) ? grayScale : 0;
		byte_to_send |= (data[(j+1)/8] & (1<<((j+1)%8))) ? (grayScale<<4) : 0;

		dsp_writeDisplayCmd(byte_to_send);

	}
}

sFONT *dsp_getFontDescriptor(uint8_t size){
	sFONT *currentFont;
	switch(size){
	case 8:
		currentFont = &Font8;
		break;
	case 12:
		currentFont = &Font12;
		break;
	case 16:
		currentFont = &Font16;
		break;
	case 20:
		currentFont = &Font20;
		break;
	case 24:
		currentFont = &Font24;
		break;
	}
	return currentFont;
}

void dsp_printCharacterToMemory(uint8_t *data, uint8_t font, uint8_t base_x, uint8_t base_y, uint8_t char_offset) {
	int cy, cx;
	sFONT *currentFont;
	currentFont = dsp_getFontDescriptor(font);

	for(cy=0; cy<currentFont->Height ; cy++){
		for(cx=0; cx<currentFont->Width; cx++){
			if(currentFont->table[8*char_offset+cy] & (0x80>>cx)) {
				dsp_setPixelToMemory(data,base_x+cx,base_y+cy);
			} else {
				dsp_clearPixelFromMemory(data,base_x+cx,base_y+cy);
			}
		}
	}
}
void dsp_printLineToMemory(uint8_t *data, uint8_t font, uint8_t base_x, uint8_t base_y, uint8_t *string, uint8_t string_len){
	int i;
	sFONT *currentFont;
	currentFont = dsp_getFontDescriptor(font);
	for(i=0;i<string_len;i++){
		dsp_printCharacterToMemory(data, font, base_x+i*currentFont->Width, base_y, dsp_calculateCharTableOffsetOfChar(string[i]));
	}
}

uint32_t dsp_calculateCharTableOffsetOfChar(uint8_t character) {
	int32_t offset = character-0x20;
	if(offset<0) return 0;
	if(offset>0x5e) return 0x5e;
	return offset;
}

uint8_t text[] = "Logan Wheat went out on a small boat to check on cattle and ended up capturing one of the most startling photos of flooding from Harvey.\
		What used to be Interstate 10, south of Beaumont, Texas, looked like an ocean, with waves lapping.\
		This split screen shows what that stretch of I-10 looked like before Harvey hit -- and what it looks like now.\
		The wind-churned waves almost tipped Wheat's boat over, Wheat told CNN.\
		The boat was being thrown around a lot, he said.\
		The waves, he added, were anywhere from 3 to 4 feet.";

void dsp_displayInitWithCommands(void){
	HAL_Display_RESET(0);
	osDelay(1);
	HAL_Display_CorD(DISPLAY_COMMAND);
	osDelay(1);
	dsp_writeDisplayCmd(0xfd); //set command lock
	dsp_writeDisplayCmd(0x12); //                 - unlock
	dsp_writeDisplayCmd(0xae); //display off
	dsp_writeDisplayCmd(0x15); //set column start and end
	dsp_writeDisplayCmd(0x00); //                         - start
	dsp_writeDisplayCmd(0x7f); //                         - end
	dsp_writeDisplayCmd(0x75); //set row start and end
	dsp_writeDisplayCmd(0x00); //                         - start
	dsp_writeDisplayCmd(0x1f); //                         - end
	dsp_writeDisplayCmd(0x81); //set contrast current
	dsp_writeDisplayCmd(0x27); //                         - 0x00~0xff
	dsp_writeDisplayCmd(0x87); //set current range - full
	dsp_writeDisplayCmd(0xa0); //set remap and grayscale mode
	dsp_writeDisplayCmd(0x02); //        dsp_writeDisplayCmd(0x07);                     -
	dsp_writeDisplayCmd(0xa1);
	dsp_writeDisplayCmd(0x00);
	dsp_writeDisplayCmd(0xa2);
	dsp_writeDisplayCmd(0x00);
	dsp_writeDisplayCmd(0xa8);
	dsp_writeDisplayCmd(0x1f);
	dsp_writeDisplayCmd(0xb1);
	dsp_writeDisplayCmd(0x71);
	dsp_writeDisplayCmd(0xb3);
	dsp_writeDisplayCmd(0xf0);
	dsp_writeDisplayCmd(0xb7);
	dsp_writeDisplayCmd(0xbb);
	dsp_writeDisplayCmd(0x35);
	dsp_writeDisplayCmd(0xff);
	dsp_writeDisplayCmd(0xbc);
	dsp_writeDisplayCmd(0x1f);
	dsp_writeDisplayCmd(0xbe);
	dsp_writeDisplayCmd(0x0f);
	dsp_writeDisplayCmd(0xaf);
}
TaskStatus_t task_4;

void displayTask(void const * argument) {

	dsp_displayInitWithCommands();
	//dsp_writeDisplayCmd(0xa6);
	HAL_Display_CorD(DISPLAY_DATA);
	int x=0,y=0, x1=100, y1=10, i=0;
	int xdir=0, ydir=0, x1dir=1, y1dir=0;

	dsp_printLineToMemory(displayData, 8, 0, 0, "Hello!", 6);

	dsp_text_InitTextbox( &(g_textbox_list[1]), 16, 150, 8, 10, 8, 5 );
	dsp_text_setText( &(g_textbox_list[1]), text, 0);




	while(1){
		i++;
		if(i==20) {
			g_textbox_list[1].linePointer_nr++;
			i=0;
		}
		if(g_textbox_list[1].linePointer_nr==10) {
			g_textbox_list[1].linePointer_nr=0;
		}

		vTaskGetInfo( xTaskGetCurrentTaskHandle(), &task_4, 1, eRunning);
		dsp_txt_printTBToMemory(&(g_textbox_list[1]), displayData);
		dsp_writeDataToDisplay(displayData, displayGrayscale);
		osDelay(100);

	}

}



void dsp_text_InitTextbox( dsp_textbox_t *tb, uint8_t height, uint8_t width, uint8_t x_0_pos, uint8_t y_0_pos, uint8_t font, uint8_t grayscale ) {
	tb->height_px = height;
	tb->width_px = width;
	tb->x_pos_px = x_0_pos;
	tb->y_pos_px = y_0_pos;
	tb->charFont = font;
	tb->charGrayscale = grayscale;
}

void dsp_text_setText( dsp_textbox_t *tb, uint8_t *data, uint32_t line_ptr) {
	tb->textData_char = data;
	tb->linePointer_nr = 0;
}

void dsp_txt_printTBToMemory(dsp_textbox_t *tb, uint8_t *displayData) {
	uint8_t char_height,char_width, visible_lines, visible_characters;
	sFONT *currentFont;
	int i;
	currentFont = dsp_getFontDescriptor(tb->charFont);
	char_height = currentFont->Height;
	char_width = currentFont->Width;
	visible_lines = tb->height_px / char_height;
	visible_characters = tb->width_px / char_width;
	for(i=0;i<visible_lines; i++) {
		dsp_printLineToMemory(displayData, tb->charFont, tb->x_pos_px, tb->y_pos_px + i*char_height, tb->textData_char + tb->linePointer_nr*visible_characters + i*visible_characters, visible_characters);
	}
}







