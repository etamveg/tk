/*
 * display.h
 *
 *  Created on: 2017. aug. 30.
 *      Author: Lenovo
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#define DISPLAY_SELECTED 1
#define DISPLAY_NOT_SELECTED 0

#define DISPLAY_COMMAND 0
#define DISPLAY_DATA 1

#define DSP_OBJECT_TEXTBOX	1
#define DSP_TEXTBOX_MAX_TEXT_LENGTH 1024
#include <stdint.h>
extern uint8_t displayData[];



typedef struct dsp_textbox_s {
	uint8_t structTypeX;
	uint8_t *textBoxData;
	uint8_t height_px;
	uint8_t width_px;
	uint8_t x_pos_px;
	uint8_t y_pos_px;

	uint8_t *textData_char;
	uint32_t textData_length;
	uint32_t charOffset_px;

	uint8_t lineOffset_px;

	uint8_t charFont;
	uint8_t charGrayscale;

} dsp_textbox_t;

extern dsp_textbox_t g_textbox_list[];

void displayTask(void const * argument);
void dsp_printCharacterToMemory(uint8_t *data, uint8_t font, uint8_t base_x, uint8_t base_y, uint8_t char_offset);
void dsp_printLineToMemory(uint8_t *data, uint8_t font, uint8_t base_x, uint8_t base_y, uint8_t *string, uint8_t string_len);

void dsp_txt_printTBToMemory(dsp_textbox_t *tb, uint8_t *displayData);
void dsp_text_setText( dsp_textbox_t *tb, uint8_t *data, uint32_t line_ptr);
void dsp_text_InitTextbox( dsp_textbox_t *tb, uint8_t height, uint8_t width, uint8_t x_0_pos, uint8_t y_0_pos, uint8_t font, uint8_t grayscale );
uint8_t dsp_getTextboxPixel(dsp_textbox_t *tb, uint8_t x, uint8_t y);
void dsp_setPixelInTextbox(dsp_textbox_t *tb, uint8_t x, uint8_t y);
void dsp_clearPixelInTextbox(dsp_textbox_t *tb, uint8_t x, uint8_t y);
void dsp_fillTextBoxWithText(dsp_textbox_t *tb);
void dsp_scrollTexboxRelative(dsp_textbox_t *tb, int32_t x_px, int32_t y_px);
void dsp_scrollTexboxAbsolute(dsp_textbox_t *tb, int32_t x_px, int32_t y_px);
void dsp_cleanDisplayData(uint8_t *data);
#endif /* DISPLAY_H_ */
