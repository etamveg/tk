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
#include <stdlib.h>
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

	dsp_printLineToMemory(displayData, 8, 0, 0, (uint8_t*)"Hello!", 6);

	dsp_text_InitTextbox( &(g_textbox_list[0]), 24, 150, 8, 8, 12, 5 );

	dsp_text_InitTextbox( &(g_textbox_list[1]), 30, 90, 160, 1, 8, 5 );

	while(1){


		vTaskGetInfo( xTaskGetCurrentTaskHandle(), &task_4, 1, eRunning);
		dsp_fillTextBoxWithText(&(g_textbox_list[0]));
		dsp_txt_printTBToMemory(&(g_textbox_list[0]), displayData);
		dsp_fillTextBoxWithText(&(g_textbox_list[1]));
		dsp_txt_printTBToMemory(&(g_textbox_list[1]), displayData);
		dsp_writeDataToDisplay(displayData, displayGrayscale);
		osDelay(1);

	}

}



void dsp_text_InitTextbox( dsp_textbox_t *tb, uint8_t height, uint8_t width, uint8_t x_0_pos, uint8_t y_0_pos, uint8_t font, uint8_t grayscale ) {
	uint8_t textBoxDataLength;
	tb->height_px = height;
	tb->width_px = width;
	tb->x_pos_px = x_0_pos;
	tb->y_pos_px = y_0_pos;
	tb->charFont = font;
	tb->charGrayscale = grayscale;
	textBoxDataLength = tb->height_px*tb->width_px/8;
	if((tb->height_px*tb->width_px)%8) {
		textBoxDataLength++;
	}
	tb->textBoxData = malloc(textBoxDataLength);
}

void dsp_text_DeleteTextbox(dsp_textbox_t *tb) {
	free(tb->textBoxData);
}

void dsp_text_setText( dsp_textbox_t *tb, uint8_t *data, uint32_t dataLength) {
	tb->textData_char = data;
	if(dataLength == 0) {
		int i;
		for(i=0;i<DSP_TEXTBOX_MAX_TEXT_LENGTH; i++){
			if(data[i] == '\0') {
				tb->textData_length = i;
				return;
			}
		}
		tb->textData_length = DSP_TEXTBOX_MAX_TEXT_LENGTH;
	} else {
		tb->textData_length = dataLength;
	}
}

void dsp_txt_printTBToMemory(dsp_textbox_t *tb, uint8_t *displayData) {
	int cx,cy;
	for(cy=0; cy<tb->height_px ; cy++){
		for(cx=0; cx<tb->width_px; cx++){
			if(dsp_getTextboxPixel(tb, cx, cy)) {
				dsp_setPixelToMemory(displayData,tb->x_pos_px+cx,tb->y_pos_px+cy);
			} else {
				dsp_clearPixelFromMemory(displayData,tb->x_pos_px+cx,tb->y_pos_px+cy);
			}
		}
	}


}

void dsp_fillTextBoxWithText(dsp_textbox_t *tb) {
	int cy, cx, p_startChr, firstLineYOffset, firstCharXOffset, pcharacterToDisplay, charPerLine;
	uint8_t characterToDisplay;
	uint8_t font_byteWidth;
	uint8_t i;
	uint32_t characterBitmap, p_fontTable, bitmapCheckPattern;
	sFONT *currentFont;

	currentFont = dsp_getFontDescriptor(tb->charFont);

	if( currentFont->Width <= 8 ) {
		font_byteWidth = 1;
	} else if( currentFont->Width <= 16 && currentFont->Width > 8  ) {
		font_byteWidth = 2;
	} else if ( currentFont->Width <= 24 && currentFont->Width > 16 ) {
		font_byteWidth = 3;
	}

	bitmapCheckPattern = 1<<(font_byteWidth*8-1);

	charPerLine = tb->width_px/currentFont->Width;
	p_startChr = tb->lineOffset_px/currentFont->Height * charPerLine + tb->charOffset_px/currentFont->Width;
	firstLineYOffset = tb->lineOffset_px%currentFont->Height;
	firstCharXOffset = tb->charOffset_px%currentFont->Width;


	for(cy=0; cy<tb->height_px ; cy++){
		for(cx=0; cx<tb->width_px; cx++){


			pcharacterToDisplay = p_startChr + (cx+firstCharXOffset)/currentFont->Width + (cy+firstLineYOffset)/currentFont->Height*charPerLine;
			if(pcharacterToDisplay < tb->textData_length){
				characterToDisplay = tb->textData_char[pcharacterToDisplay];
			} else {
				characterToDisplay = ' ';
			}

			characterBitmap=0;

			p_fontTable = currentFont->Height*dsp_calculateCharTableOffsetOfChar(characterToDisplay)*font_byteWidth+font_byteWidth*(cy+firstLineYOffset)%(currentFont->Height*font_byteWidth);

			for(i=0; i<font_byteWidth; i++){
				characterBitmap = characterBitmap<<8;
				characterBitmap |= currentFont->table[p_fontTable+i];
			}

			if( characterBitmap & (bitmapCheckPattern>>(cx+firstCharXOffset)%currentFont->Width)) {
				dsp_setPixelInTextbox(tb,cx,cy);
			} else {
				dsp_clearPixelInTextbox(tb,cx,cy);
			}
		}
	}
}

void dsp_setPixelInTextbox(dsp_textbox_t *tb, uint8_t x, uint8_t y){

	uint32_t pos;
	if(x>=tb->width_px || y>=tb->height_px) {
		return;
	}
	pos = x+y*tb->width_px;
	tb->textBoxData[pos/8] |= (1 << (pos%8));

}
void dsp_clearPixelInTextbox(dsp_textbox_t *tb, uint8_t x, uint8_t y){

	uint32_t pos;
	if(x>=tb->width_px || y>=tb->height_px) {
		return;
	}
	pos = x+y*tb->width_px;
	tb->textBoxData[pos/8] &= ~(1 << (pos%8));

}
uint8_t dsp_getTextboxPixel(dsp_textbox_t *tb, uint8_t x, uint8_t y){
	uint32_t pos;
	if(x>=tb->width_px || y>=tb->height_px) {
		return 0;
	}
	pos = x+y*tb->width_px;
	return tb->textBoxData[pos/8] & (1 << (pos%8));
}

void dsp_scrollTexboxRelative(dsp_textbox_t *tb, int32_t x_px, int32_t y_px) {
	if((int)tb->lineOffset_px + x_px >= 0) {
		tb->lineOffset_px += x_px;
	}
	if((int)tb->charOffset_px + y_px >= 0) {
		tb->charOffset_px += y_px;
	}
}

void dsp_scrollTexboxAbsolute(dsp_textbox_t *tb, int32_t x_px, int32_t y_px) {
	if(x_px >= 0) {
		tb->lineOffset_px = x_px;
	}
	if(y_px >= 0) {
		tb->charOffset_px = y_px;
	}
}
