/*
 * menu_handler.c
 *
 *  Created on: 2017. szept. 3.
 *      Author: Lenovo
 */

#include <stdlib.h>
#include <string.h>
#include "menu_handler.h"
#include "display.h"

uint8_t menu_item_selector = 0;

dsp_textbox_t item_select;
char item_select_text[10];

dsp_textbox_t start_page_a, start_page_b, start_page_c;
char start_page_file_select[] = "File select";
char start_page_file_read[] 	= "File read";
char start_page_settings[] 	= "Settings";

void menu_showMenu(menuStates_t menu) {
	dsp_cleanDisplayData(displayData);
	if(menu == MENU_START_PAGE) {
		dsp_text_InitTextbox( &start_page_a, 12, 80,   3, 10, 12, 5 );
		dsp_text_InitTextbox( &start_page_b, 12, 80,  88, 10, 12, 5 );
		dsp_text_InitTextbox( &start_page_c, 12, 80, 173, 10, 12, 5 );
		if(menu_item_selector == 0 ){
			dsp_text_InitTextbox( &item_select, 12, 80, 3, 22, 12, 5 );
		} else if(menu_item_selector == 1 ){
			dsp_text_InitTextbox( &item_select, 12, 80, 88, 22, 12, 5 );
		} else if(menu_item_selector == 2 ){
			dsp_text_InitTextbox( &item_select, 12, 80, 173, 22, 12, 5 );
		}

		dsp_text_setText( &start_page_a,start_page_file_select, 0);
		dsp_text_setText( &start_page_b,start_page_file_read, 0);
		dsp_text_setText( &start_page_c,start_page_settings, 0);
		strcpy(item_select_text, "xxxxxxxxxx");
		dsp_text_setText( &item_select,item_select_text, 0);

		dsp_fillTextBoxWithText(&start_page_a);
		dsp_fillTextBoxWithText(&start_page_b);
		dsp_fillTextBoxWithText(&start_page_c);
		dsp_fillTextBoxWithText(&item_select);

		dsp_txt_printTBToMemory(&start_page_a, displayData);
		dsp_txt_printTBToMemory(&start_page_b, displayData);
		dsp_txt_printTBToMemory(&start_page_c, displayData);
		dsp_txt_printTBToMemory(&item_select, displayData);
	}
}

menuStates_t state;
void menu_logicStateMachine(menuEvent_t event) {
	static uint8_t firstRun = 1;
	if(firstRun){
		firstRun = 0;
		state = MENU_START_PAGE;
	}

	switch(state){
		case MENU_START_PAGE:
			switch(event){
				case EVENT_UP:
					menu_item_selector = (menu_item_selector+1)%3;
					break;
				case EVENT_DOWN:
					menu_item_selector = (menu_item_selector+1)%3;
					break;
				case EVENT_EXIT:
					break;
				case EVENT_ENTER:
					switch(menu_item_selector) {
					case 0:
						state = MENU_FILE_SELECT;
						break;
					case 1:
						state = MENU_READ_TEXT;
						break;
					case 2:
						state = MENU_SETTINGS;
						break;
					}
					break;
			}
			break;
		case MENU_FILE_SELECT:
			switch(event){
				case EVENT_UP:
					break;
				case EVENT_DOWN:
					break;
				case EVENT_EXIT:
					break;
				case EVENT_ENTER:
					break;
			}
			break;
		case MENU_READ_TEXT:
			switch(event){
				case EVENT_UP:
					break;
				case EVENT_DOWN:
					break;
				case EVENT_EXIT:
					break;
				case EVENT_ENTER:
					break;
			}
			break;
		case MENU_SETTINGS:
			switch(event){
				case EVENT_UP:
					break;
				case EVENT_DOWN:
					break;
				case EVENT_EXIT:
					break;
				case EVENT_ENTER:
					break;
			}
			break;
	}

	menu_showMenu(state);


}

void menu_upEvent(uint32_t duration) {
	menu_logicStateMachine(EVENT_UP);
}
void menu_downEvent(uint32_t duration) {
	menu_logicStateMachine(EVENT_DOWN);
}
void menu_enterEvent(uint32_t duration) {
	menu_logicStateMachine(EVENT_ENTER);
}
void menu_exitEvent(uint32_t duration) {
	menu_logicStateMachine(EVENT_EXIT);
}
