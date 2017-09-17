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
uint8_t displayDataChanged = 0;

dsp_textbox_t g_textbox_list[2];

void dsp_cleanDisplayData(uint8_t *data){
	int i;
	for(i=0; i<256*4;i++){
		data[i]=0;
	}
}
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
void dsp_printLineToMemory(uint8_t *data,
							uint8_t font,
							uint8_t base_x,
							uint8_t base_y,
							uint8_t *string,
							uint8_t string_len,
							uint32_t display_width,
							uint32_t display_height,
							uint8_t is_inverted){
	int cy, cx, pcharacterToDisplay, charPerLine;
	uint8_t characterToDisplay;
	uint8_t font_byteWidth;
	uint8_t i;
	uint32_t characterBitmap, p_fontTable, bitmapCheckPattern;
	sFONT *currentFont;

	currentFont = dsp_getFontDescriptor(font);

	if( currentFont->Width <= 8 ) {
		font_byteWidth = 1;
	} else if( currentFont->Width <= 16 && currentFont->Width > 8  ) {
		font_byteWidth = 2;
	} else if ( currentFont->Width <= 24 && currentFont->Width > 16 ) {
		font_byteWidth = 3;
	}

	bitmapCheckPattern = 1<<(font_byteWidth*8-1);

	charPerLine = (display_width-base_y)/currentFont->Width;


	for(cy=base_y; cy<base_y+currentFont->Height ; cy++){
		for(cx=base_x; cx<base_x+currentFont->Width*string_len; cx++){


			pcharacterToDisplay =  (cx-base_x)/currentFont->Width;
			if(pcharacterToDisplay < string_len){
				characterToDisplay = string[pcharacterToDisplay];
			} else {
				characterToDisplay = ' ';
			}

			characterBitmap=0;

			p_fontTable = currentFont->Height*dsp_calculateCharTableOffsetOfChar(characterToDisplay)*font_byteWidth+font_byteWidth*(cy)%(currentFont->Height*font_byteWidth);

			for(i=0; i<font_byteWidth; i++){
				characterBitmap = characterBitmap<<8;
				characterBitmap |= currentFont->table[p_fontTable+i];
			}
			if(is_inverted) {
				if( (characterBitmap & (bitmapCheckPattern>>(cx)%currentFont->Width))) {
					dsp_clearPixelFromMemory(data,cx,cy);
				} else {
					dsp_setPixelToMemory(data,cx,cy);
				}
			} else {
				if( (characterBitmap & (bitmapCheckPattern>>(cx)%currentFont->Width))) {
					dsp_setPixelToMemory(data,cx,cy);
				} else {
					dsp_clearPixelFromMemory(data,cx,cy);
				}
			}
		}
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
	int i = 0;
	osDelay(500);
	dsp_displayInitWithCommands();
	//dsp_writeDisplayCmd(0xa6);
	HAL_Display_CorD(DISPLAY_DATA);

	while(1){


		vTaskGetInfo( xTaskGetCurrentTaskHandle(), &task_4, 1, eRunning);


		if(displayDataChanged || i >= 10){
			displayDataChanged = 0;
			i=0;
			dsp_writeDataToDisplay(displayData, displayGrayscale);
		}

		osDelay(1);
		i++;

	}

}

void dsp_text_InitTextbox( dsp_textbox_t *tb, uint8_t height, uint8_t width, uint8_t x_0_pos, uint8_t y_0_pos, uint8_t font, uint8_t grayscale ) {
	uint32_t textBoxDataLength;
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
	tb->pixelInverted = 0;
	tb->textBoxData = malloc(textBoxDataLength);
}
void dsp_text_DeleteTextbox(dsp_textbox_t *tb) {
	tb->structTypeX=0;
	tb->height_px=0;
	tb->width_px=0;
	tb->x_pos_px=0;
	tb->y_pos_px=0;

	tb->textData_char=0;
	tb->textData_length=0;
	tb->charOffset_px=0;

	tb->lineOffset_px=0;

	tb->charFont=0;
	tb->charGrayscale=0;
	tb->pixelInverted = 0;
	free(tb->textBoxData);
	tb->textBoxData=0;
}
void dsp_text_setText( dsp_textbox_t *tb, uint8_t *data, uint32_t dataLength) {
	tb->textData_char = data;
	if(dataLength == 0) {
		int i;
		for(i=0;i<DSP_TEXTBOX_MAX_TEXT_LENGTH; i++){
			if(data[i] == '\0') {
				tb->textData_length = i;
				displayDataChanged=1;
				return;
			}
		}
		tb->textData_length = DSP_TEXTBOX_MAX_TEXT_LENGTH;
	} else {
		tb->textData_length = dataLength;
	}
	displayDataChanged=1;
}
void dsp_setTbPosition(dsp_textbox_t *tb, uint16_t xpos, uint16_t ypos) {
	tb->x_pos_px = xpos;
	tb->y_pos_px = ypos;
}
void dsp_setTbInversion(dsp_textbox_t *tb, uint8_t enable) {
	if(enable){
		tb->pixelInverted = 1;
	} else {
		tb->pixelInverted = 0;
	}
}
uint8_t dsp_getTbInversion(dsp_textbox_t *tb) {
	return tb->pixelInverted = 1;
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
	if(tb->pixelInverted) {
		return !(tb->textBoxData[pos/8] & (1 << (pos%8)));
	} else {
		return tb->textBoxData[pos/8] & (1 << (pos%8));
	}
}
void dsp_scrollTexboxRelative(dsp_textbox_t *tb, int32_t x_px, int32_t y_px) {
	if((int)tb->lineOffset_px + x_px >= 0) {
		tb->lineOffset_px += x_px;
	}
	if((int)tb->charOffset_px + y_px >= 0) {
		tb->charOffset_px += y_px;
	}
	displayDataChanged=1;
}
void dsp_scrollTexboxAbsolute(dsp_textbox_t *tb, int32_t x_px, int32_t y_px) {
	if(x_px >= 0) {
		tb->lineOffset_px = x_px;
	}
	if(y_px >= 0) {
		tb->charOffset_px = y_px;
	}
	displayDataChanged=1;
}
uint32_t dsp_getTextboxWidthInChar(dsp_textbox_t *tb) {
	sFONT *font = dsp_getFontDescriptor(tb->charFont);
	return tb->width_px/font->Width;
}

void dsp_sb_initScrollBar(dsp_scroll_bar_t *sb, uint8_t height, uint8_t width, uint8_t x_0_pos, uint8_t y_0_pos, float bar_pos, float bar_size, uint8_t inverted){
	uint32_t dataBufferSize;
	sb->height_px = height;
	sb->width_px = width;
	sb->x_pos_px = x_0_pos;
	sb->y_pos_px = y_0_pos;
	sb->pixelInverted = inverted;
	if(bar_pos<=1.0 && bar_pos>=0) {
		sb->scrollPadRelativePosition = bar_pos;
	} else {
		sb->scrollPadRelativePosition = 0;
	}
	if(bar_size<=1.0 && bar_size>=0){
		sb->scrollPadRelativeSize = bar_size;
	} else {
		sb->scrollPadRelativeSize = 0.1;
	}
	dataBufferSize = sb->width_px * sb->width_px / 8;
	if((sb->width_px * sb->width_px)%8) {
		dataBufferSize++;
	}
	sb->scrollBarDspData = malloc( dataBufferSize );
}
void dsp_sb_deleteScrollBar(dsp_scroll_bar_t *sb) {
	sb->height_px = 0;
	sb->width_px = 0;
	sb->x_pos_px = 0;
	sb->y_pos_px = 0;
	sb->scrollPadRelativePosition = 0;
	sb->scrollPadRelativeSize = 0;
	free(sb->scrollBarDspData);
}
void dsp_sb_setBarSize(dsp_scroll_bar_t *sb, float size) {
	if(size<=1.0 && size>=0){
		sb->scrollPadRelativeSize = size;
	} else {
		sb->scrollPadRelativeSize = 0.1;
	}
}
void dsp_sb_setBarPosition(dsp_scroll_bar_t *sb, float pos) {
	if(pos<=1.0 && pos>=0) {
		sb->scrollPadRelativePosition = pos;
	} else {
		sb->scrollPadRelativePosition = 0;
	}
}
void dsp_sb_setPixelInvert(dsp_scroll_bar_t *sb, uint8_t enable){
	sb->pixelInverted = enable;
}
void dsp_sb_setPixel(dsp_scroll_bar_t *sb, uint8_t x, uint8_t y){

	uint32_t pos;
	if(x>=sb->width_px || y>=sb->height_px) {
		return;
	}
	pos = x+y*sb->width_px;
	sb->scrollBarDspData[pos/8] |= (1 << (pos%8));

}
void dsp_sb_clearPixel(dsp_scroll_bar_t *sb, uint8_t x, uint8_t y){

	uint32_t pos;
	if(x>=sb->width_px || y>=sb->height_px) {
		return;
	}
	pos = x+y*sb->width_px;
	sb->scrollBarDspData[pos/8] &= ~(1 << (pos%8));

}
uint8_t dsp_sb_getScrollBarImagePixel(dsp_scroll_bar_t *sb, uint8_t x, uint8_t y) {
	uint32_t pos;
	if(x>=sb->width_px || y>=sb->height_px) {
		return 0;
	}
	pos = x+y*sb->width_px;
	if(sb->pixelInverted) {
		return !(sb->scrollBarDspData[pos/8] & (1 << (pos%8)));
	} else {
		return sb->scrollBarDspData[pos/8] & (1 << (pos%8));
	}
}
void dsp_sb_calculateDisplayImage( dsp_scroll_bar_t *sb ) {
	uint32_t bar_length = 0;
	uint32_t bar_position = 0;
	bar_length = sb->width_px * sb->scrollPadRelativeSize;
	if(bar_length < 2) {
		bar_length = 2;
	}
	bar_position = sb->width_px * sb->scrollPadRelativePosition;
	if(bar_position > sb->width_px-bar_length){
		bar_position = sb->width_px-bar_length;
	}
	int y, x;

	for(y=0;y<sb->height_px;y++) {
		for(x=0; x<sb->width_px;x++){
			if(x<bar_position || x>bar_position+bar_length) {
				dsp_sb_clearPixel(sb, x, y);
			} else {
				dsp_sb_setPixel(sb, x, y);
			}
		}
	}
}
void dsp_sb_printScrollbarToDisplayData( dsp_scroll_bar_t *sb, uint8_t *displayData ) {
	int cx,cy;
	dsp_sb_calculateDisplayImage(sb);
	for(cy=0; cy<sb->height_px ; cy++){
		for(cx=0; cx<sb->width_px; cx++){
			if(dsp_sb_getScrollBarImagePixel(sb, cx, cy)) {
				dsp_setPixelToMemory(displayData,sb->x_pos_px+cx,sb->y_pos_px+cy);
			} else {
				dsp_clearPixelFromMemory(displayData,sb->x_pos_px+cx,sb->y_pos_px+cy);
			}
		}
	}
}


